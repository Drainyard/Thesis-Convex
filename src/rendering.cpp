#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static void SetMatrixUniform(GLuint programID, const char* name, glm::mat4 matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, &matrix[0][0]);
}

static void SetVec3Uniform(GLuint programID, const char* name, glm::vec3 vec)
{
    glUniform3f(glGetUniformLocation(programID, name), vec.x, vec.y, vec.z);
}

static void SetVec4Uniform(GLuint programID, const char* name, glm::vec4 vec)
{
    glUniform4f(glGetUniformLocation(programID, name), vec.x, vec.y, vec.z, vec.w);
}

static void SetVec3Uniform(GLuint programID, const char* name, glm::vec4 vec)
{
    glUniform3f(glGetUniformLocation(programID, name), vec.x, vec.y, vec.z);
}

static void SetVec2Uniform(GLuint programID, const char* name, glm::vec2 vec)
{
    glUniform2f(glGetUniformLocation(programID, name), vec.x, vec.y);
}

static void SetFloatUniform(GLuint programID, const char* name, float value)
{
    glUniform1f(glGetUniformLocation(programID, name), value);
}

static vertex* LoadObj(const char* filePath)
{
    vertex* vertices = nullptr;
    auto file = fopen(filePath, "r");
    if(file)
    {
        int vCount = 0;
        char buffer[64];
        
        while(fgets(buffer, 64, file))
        {
            if(startsWith(buffer, "v"))
            {
                vCount++;
            }
        }
        
        vertices = (vertex*)calloc(vCount, sizeof(vertex));
        
        rewind(file);
        
        int i = 0;
        
        while(fgets(buffer, 64, file))
        {
            if(startsWith(buffer, "v"))
            {
                sscanf(buffer, "v %f %f %f", &vertices[i].position.x, &vertices[i].position.y, &vertices[i].position.z);
                
                vertices[i].position = vertices[i].position;
                vertices[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
                i++;
            }
        }
        
        fclose(file);
    }
    
    return vertices;
}

static vertex* CopyVertices(vertex* vertices, int numberOfPoints)
{
    auto res = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++)
    {
        res[i].position = vertices[i].position;
        res[i].color = vertices[i].color;
    }
    return res;
}

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam)
{
    (void)userParam; // Silence unused warning
    if(type == GL_DEBUG_TYPE_ERROR)
    {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s, source = %x, id = %ud, length %ud= \n",
                (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
                type, severity, message, source, id, length);
    }
}

static glm::mat4 ComputeTransformation(glm::vec3 scale = glm::vec3(1.0f), glm::quat orientation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f), glm::vec3 position = glm::vec3(0.0f))
{
    glm::mat4 t;
    t = glm::scale(glm::mat4(1.0f), scale);
    t = glm::toMat4(orientation) * t;
    t = glm::translate(t, position);
    return t;
}

int* FindExtremePoints(vertex* points, int numPoints)
{
    
    if(numPoints > 0)
    {
        auto minX = 0;
        auto maxX = 0;
        auto minY = 0;
        auto maxY = 0;
        auto minZ = 0;
        auto maxZ = 0;
        
        for(int i = 0; i < numPoints; i++)
        {
            auto& p = points[i];
            if(p.position.x > points[maxX].position.x)
            {
                maxX = i;
            }
            if(p.position.x < points[minX].position.x)
            {
                minX = i;
            }
            if(p.position.y > points[maxY].position.y)
            {
                maxY = i;
            }
            if(p.position.y < points[minY].position.y)
            {
                minY = i;
            }
            if(p.position.z > points[maxZ].position.z)
            {
                maxZ = i;
            }
            if(p.position.z < points[minZ].position.z)
            {
                minZ = i;
            }
        }
        int* res = (int*)malloc(sizeof(int) * 6);
        res[0] = minX;
        res[1] = maxX;
        res[2] = minY;
        res[3] = maxY;
        res[4] = minZ;
        res[5] = maxZ;
        return res;
    }
    return nullptr;
}

static void ComputeMeshTransformation(mesh& object)
{
    object.transform = ComputeTransformation(object.scale, object.orientation, object.position);
}

static char* LoadShaderFromFile(const char* path)
{
    GLchar* source = nullptr;
    
    FILE* file = fopen(path, "r");
    if(file)
    {
        if(fseek(file, 0L, SEEK_END) == 0)
        {
            auto bufSize = ftell(file);
            if(bufSize == -1) 
            {
                return source;
            }
            
            source = (GLchar*)malloc(sizeof(GLchar) * (bufSize + 1));
            
            if(fseek(file, 0L, SEEK_SET) != 0)
            {
                log_a("Error reading file");
                free(source);
                return nullptr;
            }
            
            auto newLen = fread(source, sizeof(GLchar), bufSize, file);
            if(ferror(file) != 0)
            {
                log_a("Error reading file");
            }
            else
            {
                source[newLen++] = '\0';
            }
        }
        fclose(file);
    }
    
    return source;
}

static shader LoadShaders(const char* vertexFilePath, const char* fragmentFilePath)
{
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    auto vertexShaderCode = LoadShaderFromFile(vertexFilePath);
    auto fragmentShaderCode = LoadShaderFromFile(fragmentFilePath);
    if(!vertexShaderCode)
    {
        fprintf(stderr, "Could not load vertex shader: %s\n", vertexFilePath);
    }
    if(!fragmentShaderCode)
    {
        fprintf(stderr, "Could not load fragment shader: %s\n", fragmentFilePath);
    }
    
    GLint result = GL_FALSE;
    int infoLogLength;
    
    glShaderSource(vertexShaderID, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShaderID);
    
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0)
    {
        char* Buffer = (char*)malloc((infoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &Buffer[0]);
        printf("%s\n", &Buffer[0]);
    }
    
    
    glShaderSource(fragmentShaderID, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShaderID);
    
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0)
    {
        char* buffer = (char*)malloc((infoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &buffer[0]);
        printf("%s\n", &buffer[0]);
    }
    
    printf("Linking program: %s\n", vertexFilePath);
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);
    
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0)
    {
        char* buffer = (char*)malloc((infoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(programID, infoLogLength, NULL, &buffer[0]);
        printf("%s\n", &buffer[0]);
        printf("%d\n", result);
    }
    
    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    free(vertexShaderCode);
    free(fragmentShaderCode);
    
    shader newShader = {};
    newShader.programID = programID;
    return newShader;
}

static void InitializeOpenGL(render_context& renderContext)
{
    if(!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(0);
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 
    
    // Open a window and create its OpenGL context
    // (In the accompanying source code, this variable is global for simplicity)
    renderContext.screenWidth= 1600;
    renderContext.screenHeight = 900;
    renderContext.window = glfwCreateWindow(renderContext.screenWidth, renderContext.screenHeight, "Convex Hull", NULL, NULL);
    if(renderContext.window == NULL){
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit(0);
    }
    
    glfwMakeContextCurrent(renderContext.window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    glfwSetCursorPosCallback(renderContext.window, MousePositionCallback);
    glfwSetScrollCallback(renderContext.window, MouseScrollCallback);
    glfwSetKeyCallback(renderContext.window, KeyCallback);
    glfwSetMouseButtonCallback(renderContext.window, MouseButtonCallback);
    
    log("%s\n", glGetString(GL_VERSION));
    log("Shading language supported: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    log("Glad Version: %d.%d\n", GLVersion.major, GLVersion.minor);
    
    glEnable(GL_DEPTH_TEST);
    
    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable debug output
    glEnable              (GL_DEBUG_OUTPUT);
    glDebugMessageCallback((GLDEBUGPROC) MessageCallback, 0);
    
    glfwSetInputMode(renderContext.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    
    renderContext.textureShader = LoadShaders("../shaders/texture.vert", "../shaders/texture.frag");
    renderContext.colorShader = LoadShaders("../shaders/color.vert", "../shaders/color.frag");
    renderContext.basicShader = LoadShaders("../shaders/basic.vert", "../shaders/basic.frag");
    renderContext.particleShader = LoadShaders("../shaders/particle.vert", "../shaders/particle.frag");
    
    // Initialize line buffer
    renderContext.lineShader = LoadShaders("../shaders/lineShader.vert", "../shaders/lineShader.frag");
    glGenVertexArrays(1, &renderContext.lineVAO);
    glBindVertexArray(renderContext.lineVAO);
    glGenBuffers(1, &renderContext.lineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.lineVBO);
    glGenBuffers(1, &renderContext.lineEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderContext.lineEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * LINE_INDICES, renderContext.lineIndices, GL_STATIC_DRAW);
    
    
    glBindVertexArray(0);
    
    
    // Initialize Quad buffers
    glGenVertexArrays(1, &renderContext.quadVAO);
    glBindVertexArray(renderContext.quadVAO);
    
    glGenBuffers(1, &renderContext.quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderContext.quadVertices),renderContext.quadVertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &renderContext.quadIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderContext.quadIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(renderContext.quadIndices), renderContext.quadIndices, GL_STATIC_DRAW);
    
    glUseProgram(renderContext.basicShader.programID);
    
    auto pos = glGetAttribLocation(renderContext.basicShader.programID, "position");
    
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBindVertexArray(0);
    
    // Initialize point cloud
    glGenVertexArrays(1, &renderContext.pointCloudVAO);
    glBindVertexArray(renderContext.pointCloudVAO);
    
    static const GLfloat vbData[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
    };
    
    glGenBuffers(1, &renderContext.billboardVBO);
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.billboardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vbData), vbData, GL_STATIC_DRAW);
    glGenBuffers(1, &renderContext.particlesVBO);
    glGenBuffers(1, &renderContext.particlesColorVBO);
}

static void RenderLine(render_context& renderContext, glm::vec3 start = glm::vec3(0.0f), glm::vec3 end = glm::vec3(0.0f), glm::vec4 color = glm::vec4(1.0f), float lineWidth = 2.0f)
{
    glUseProgram(renderContext.lineShader.programID);
    
    glBindVertexArray(renderContext.lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.lineVBO);
    
    auto width = 0.21f * lineWidth;
    
    // Double vertices
    // Vertex                     Next                  
    GLfloat points[40] = {
        start.x, start.y, start.z, start.x, start.y, start.z, end.x, end.y, end.z,  1.0f,
        start.x, start.y, start.z, start.x, start.y, start.z, end.x, end.y, end.z, -1.0f,
        end.x, end.y, end.z, start.x, start.y, start.z, end.x, end.y, end.z,        1.0f,
        end.x, end.y, end.z, start.x, start.y, start.z, end.x, end.y, end.z,       -1.0f};
    
    glBufferData(GL_ARRAY_BUFFER, 40 * sizeof(GLfloat), &points[0], GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)0); // pos
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 3)); // nextScreen
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 6));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 9));
    
    auto m = glm::scale(glm::mat4(1.0f), glm::vec3(globalScale));
    
    SetMatrixUniform(renderContext.lineShader.programID, "model", m);
    SetMatrixUniform(renderContext.lineShader.programID, "view", renderContext.viewMatrix);
    SetMatrixUniform(renderContext.lineShader.programID, "projection", renderContext.projectionMatrix);
    SetVec4Uniform(renderContext.lineShader.programID, "color", color);
    SetFloatUniform(renderContext.lineShader.programID, "thickness", width);
    
    auto screenSize = glm::vec2((float)renderContext.screenWidth, (float)renderContext.screenHeight);
    
    float aspect = screenSize.x / screenSize.y;
    
    SetFloatUniform(renderContext.lineShader.programID, "aspect", aspect);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderContext.lineEBO);
    glDrawElements(GL_TRIANGLES, LINE_INDICES, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
    glBindVertexArray(0);
    //glDisableVertexAttribArray(0);
    //glDisableVertexAttribArray(1);
    //glDisableVertexAttribArray(2);
    
}

static light& CreateLight(render_context& renderContext, glm::vec3 position = glm::vec3(0.0f), glm::vec3 color = glm::vec3(1.0f), float power = 2.0f)
{
    auto& l = renderContext.lights[renderContext.lightCount++];
    
    l.position = position;
    l.color = color;
    l.power = power;
    
    return l;
}

static GLfloat* BuildVertexBuffer(face* faces, int numFaces)
{
    GLfloat* vertices = (GLfloat*)malloc(numFaces * 3 * sizeof(vertex_info));
    
    log("Num faces in renderer: %d\n", numFaces);
    
    for(int i = 0; i < numFaces; i++)
    {
        auto currentColor = faces[i].faceColor;
        
        auto v1 = faces[i].vertices[0];
        auto v2 = faces[i].vertices[1];
        auto v3 = faces[i].vertices[2];
        // First vertex
        vertices[i * 30] = v1.position.x;
        vertices[i * 30 + 1] = v1.position.y;
        vertices[i * 30 + 2] = v1.position.z;
        
        // First normal
        vertices[i * 30 + 3] = faces[i].faceNormal.x;
        vertices[i * 30 + 4] = faces[i].faceNormal.y;
        vertices[i * 30 + 5] = faces[i].faceNormal.z;
        
        // First color
        vertices[i * 30 + 6] = currentColor.x;//v1.color.x;
        vertices[i * 30 + 7] = currentColor.y;//v1.color.y;
        vertices[i * 30 + 8] = currentColor.z;//v1.color.z;
        vertices[i * 30 + 9] = currentColor.w;//v1.color.w;
        
        // Second vertex
        vertices[i * 30 + 10] = v2.position.x;
        vertices[i * 30 + 11] = v2.position.y;
        vertices[i * 30 + 12] = v2.position.z;
        
        // Second normal
        vertices[i * 30 + 13] = faces[i].faceNormal.x;
        vertices[i * 30 + 14] = faces[i].faceNormal.y;
        vertices[i * 30 + 15] = faces[i].faceNormal.z;
        
        // Second color
        vertices[i * 30 + 16] = currentColor.x;//v2.color.x;
        vertices[i * 30 + 17] = currentColor.y;//v2.color.y;
        vertices[i * 30 + 18] = currentColor.z;//v2.color.z;
        vertices[i * 30 + 19] = currentColor.w;//v2.color.w;
        
        // Third vertex
        vertices[i * 30 + 20] = v3.position.x;
        vertices[i * 30 + 21] = v3.position.y;
        vertices[i * 30 + 22] = v3.position.z;
        
        // Third normal
        vertices[i * 30 + 23] = faces[i].faceNormal.x;
        vertices[i * 30 + 24] = faces[i].faceNormal.y;
        vertices[i * 30 + 25] = faces[i].faceNormal.z;
        
        // Third color
        vertices[i * 30 + 26] = currentColor.x;//v3.color.x;
        vertices[i * 30 + 27] = currentColor.y;//v3.color.y;
        vertices[i * 30 + 28] = currentColor.z;//v3.color.z;
        vertices[i * 30 + 29] = currentColor.w;//v3.color.w;
    }
    return vertices;
}

static bool IsPointOnPositiveSide(face& f, vertex& v, coord_t epsilon = 0.0f)
{
    auto d = glm::dot(f.faceNormal, v.position - f.centerPoint);
    return d > epsilon;
}

static bool IsPointOnPositiveSide(face& f, glm::vec3 v, coord_t epsilon = 0.0f)
{
    auto d = glm::dot(f.faceNormal, v - f.centerPoint);
    return d > epsilon;
}

static glm::vec3 ComputeFaceNormal(face f)
{
    // Newell's Method
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    glm::vec3 normal = glm::vec3(0.0f);
    
    for(int i = 0; i < 3; i++)
    {
        auto& current = f.vertices[i].position;
        auto& next = f.vertices[(i + 1) % 3].position;
        
        normal.x = normal.x + (current.y - next.y) * (current.z + next.z);
        normal.y = normal.y + (current.z - next.z) * (current.x + next.x);
        normal.z = normal.z + (current.x - next.x) * (current.y + next.y);
    }
    
    return glm::normalize(normal);
}

static mesh& InitEmptyMesh(render_context& renderContext, int meshIndex = -1)
{
    if(meshIndex != -1)
    {
        auto& m = renderContext.meshes[meshIndex];
        //free(m.faces);
        m.dirty = true;
        return m;
    }
    else
    {
        mesh& object = renderContext.meshes[renderContext.meshCount++];
        object.meshIndex = renderContext.meshCount - 1;
        
        glGenVertexArrays(1, &object.VAO);
        glBindVertexArray(object.VAO);
        glGenBuffers(1, &object.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, object.VBO);
        
        auto& mat = object.material;
        mat.specularColor = glm::vec3(1.0f);
        mat.alpha = 1.0f;
        mat.type = MT_color;
        mat.diffuse.diffuseColor = glm::vec3(1.0f);
        mat.materialShader = renderContext.colorShader;
        
        object.dirty = true;
        
        glBindVertexArray(0);
        
        return object;
    }
}

static void RenderPointCloud(render_context& renderContext, vertex* inputPoints, int numPoints)
{
    glBindVertexArray(renderContext.pointCloudVAO);
    glUseProgram(renderContext.particleShader.programID);
    
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.particlesVBO);
    glBufferData(GL_ARRAY_BUFFER, numPoints * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    
    GLfloat* positions = (GLfloat*)malloc(numPoints * sizeof(GLfloat) * 4);
    GLfloat* colors = (GLfloat*)malloc(numPoints * sizeof(GLfloat) * 4);
    
    for(int i = 0; i < numPoints; i++)
    {
        positions[4 * i + 0] = inputPoints[i].position.x;
        positions[4 * i + 1] = inputPoints[i].position.y;
        positions[4 * i + 2] = inputPoints[i].position.z;
        positions[4 * i + 3] = 50.0f;
        auto c = inputPoints[i].color;
        colors[4 * i + 0] = c.x;
        colors[4 * i + 1] = c.y;
        colors[4 * i + 2] = c.z;
        colors[4 * i + 3] = c.w;
    }
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, numPoints * sizeof(GLfloat) * 4, positions);
    
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.particlesColorVBO);
    glBufferData(GL_ARRAY_BUFFER, numPoints * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, numPoints * sizeof(GLfloat) * 4, colors);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.billboardVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.particlesVBO);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.particlesColorVBO);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    
    auto m = glm::scale(glm::mat4(1.0f), glm::vec3(globalScale));
    
    SetMatrixUniform(renderContext.particleShader.programID, "M", m);
    SetMatrixUniform(renderContext.particleShader.programID, "V", renderContext.viewMatrix);
    SetMatrixUniform(renderContext.particleShader.programID, "P", renderContext.projectionMatrix);
    
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numPoints);
    
    glBindVertexArray(0);
    free(positions);
    free(colors);
}

static void RenderQuad(render_context& renderContext, glm::vec3 position = glm::vec3(0.0f), glm::quat orientation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec4 color = glm::vec4(1.0f))
{
    glBindVertexArray(renderContext.quadVAO);
    glUseProgram(renderContext.basicShader.programID);
    
    auto m = ComputeTransformation(scale, orientation, position);
    
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat), &renderContext.quadVertices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    SetMatrixUniform(renderContext.basicShader.programID, "M", m);
    SetMatrixUniform(renderContext.basicShader.programID, "V", renderContext.viewMatrix);
    SetMatrixUniform(renderContext.basicShader.programID, "P", renderContext.projectionMatrix);
    SetVec4Uniform(renderContext.basicShader.programID, "c", color);
    
    
    glDrawElements(GL_TRIANGLES, sizeof(renderContext.quadIndices), GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);
}

static void RenderFaceEdges(render_context& renderContext, std::vector<edge>& edges, vertex v1, vertex v2, vertex v3, glm::vec4 edgeColor, float lineWidth)
{
    bool e1Drawn = false;
    bool e2Drawn = false;
    bool e3Drawn = false;
    
    for(const auto& e : edges)
    {
        if((e.origin == v1.vertexIndex && e.end == v2.vertexIndex) || (e.origin == v2.vertexIndex && e.end == v1.vertexIndex))
        {
            e1Drawn = true;
        }
        
        if((e.origin == v2.vertexIndex && e.end == v3.vertexIndex) || (e.origin == v3.vertexIndex && e.end == v2.vertexIndex))
        {
            e2Drawn = true;
        }
        
        if((e.origin == v1.vertexIndex && e.end == v3.vertexIndex) || (e.origin == v3.vertexIndex && e.end == v1.vertexIndex))
        {
            e3Drawn = true;
        }
    }
    
    if(!e1Drawn)
    {
        RenderLine(renderContext, v1.position, v2.position, edgeColor, lineWidth);
        edge e = {};
        e.origin = v1.vertexIndex;
        e.end = v2.vertexIndex;
        edges.push_back(e);
    }
    
    if(!e2Drawn)
    {
        RenderLine(renderContext, v2.position, v3.position, edgeColor, lineWidth);
        edge e = {};
        e.origin = v2.vertexIndex;
        e.end = v3.vertexIndex;
        edges.push_back(e);
    }
    
    if(!e3Drawn)
    {
        RenderLine(renderContext, v3.position, v1.position, edgeColor, lineWidth);
        edge e = {};
        e.origin = v1.vertexIndex;
        e.end = v3.vertexIndex;
        edges.push_back(e);
    }
    
    
}

static void RenderMesh(render_context& renderContext, mesh& m)
{
    if(m.faces.size() == 0)
    {
        return;
    }
    
    auto& material = m.material;
    glUseProgram(material.materialShader.programID);
    
    ComputeMeshTransformation(m);
    
    SetMatrixUniform(material.materialShader.programID, "M", m.transform);
    SetMatrixUniform(material.materialShader.programID, "V", renderContext.viewMatrix);
    SetMatrixUniform(material.materialShader.programID, "P", renderContext.projectionMatrix);
    
    auto lightPos = glm::vec3(1, 1, 1);
    auto lightColor = glm::vec3(1, 1, 1);
    auto lightPower = 2.0f;
    
    if(renderContext.lightCount > 0)
    {
        auto& light = renderContext.lights[0];
        lightPos = light.position;
        lightColor = light.color;
        lightPower = light.power;
    }
    
    SetVec3Uniform(material.materialShader.programID, "lightPosWorld", lightPos);
    SetVec3Uniform(material.materialShader.programID, "lightColor", lightColor);
    SetFloatUniform(material.materialShader.programID, "lightPower", lightPower);
    SetVec3Uniform(material.materialShader.programID, "specularColor", material.specularColor);
    SetFloatUniform(material.materialShader.programID, "alpha", material.alpha);
    
    if(material.type == MT_color)
    {
        SetVec3Uniform(material.materialShader.programID, "diffuseColor", material.diffuse.diffuseColor);
    }
    
    glBindVertexArray(m.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    if(m.dirty)
    {
        free(m.currentVBO);
        m.dirty = false;
        m.currentVBO = BuildVertexBuffer(m.faces.data(), (int)m.faces.size());
    }
    
    auto vertexCount = (int)m.faces.size() * 3;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_info) * vertexCount, &m.currentVBO[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_info), (void*)offsetof(vertex, position));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_info), (void*)offsetof(vertex, normal));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_info), (void*) offsetof(vertex, color));
    
    
    glDrawArrays(GL_TRIANGLES, 0, vertexCount * 10);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindVertexArray(0);
    
    if(renderContext.renderNormals)
    {
        for(size_t i = 0; i < m.faces.size(); i++)
        {
            auto& f = m.faces[i];
            
            auto normalLength = 20.0f;
            auto normalColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            
            RenderLine(renderContext, f.centerPoint, f.centerPoint + normalLength * f.faceNormal, normalColor, 2.0f);
        }
    }
}

static void RenderGrid(render_context& renderContext, glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), float lineWidth = 2.0f)
{
    float gridSize = 15.0f;
    glm::vec3 offset = renderContext.originOffset;
    for(int x = 0; x < gridSize; x++)
    {
        for(int y = 0; y < gridSize; y++)
        {
            RenderLine(renderContext, glm::vec3(x, 0.0f, y) - offset, glm::vec3(gridSize, 0.0f, y) - offset, color, lineWidth);
            RenderLine(renderContext, glm::vec3(x, 0.0f, y) - offset, glm::vec3(x, 0.0f, gridSize) - offset, color, lineWidth);
        }
    }
    
    RenderLine(renderContext, glm::vec3(gridSize, 0.0f, 0.0f) - offset, glm::vec3(gridSize, 0.0f, gridSize) - offset, color, lineWidth);
    RenderLine(renderContext, glm::vec3(0.0f, 0.0f, gridSize) - offset, glm::vec3(gridSize, 0.0f, gridSize) - offset, color, lineWidth);
}


static void Render(render_context& renderContext)
{
    for(int meshIndex = 0; meshIndex < renderContext.meshCount; meshIndex++)
    {
        RenderMesh(renderContext, renderContext.meshes[meshIndex]);
    }
    //RenderGrid(renderContext, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 1.0f);
}

static void ComputeMatrices(render_context& renderContext, double deltaTime)
{
    if(renderContext.FoV >= 1.0f && renderContext.FoV <= 45.0f)
        renderContext.FoV -= inputState.yScroll;
    if(renderContext.FoV <= 1.0f)
        renderContext.FoV = 1.0f;
    if(renderContext.FoV >= 45.0f)
        renderContext.FoV = 45.0f;
    
    renderContext.projectionMatrix = glm::perspective(glm::radians(renderContext.FoV), (float)renderContext.screenWidth / (float)renderContext.screenHeight, renderContext.nearPlane, renderContext.farPlane);
    
    float panSpeed = 60.0f * (float)deltaTime;
    if(Key(Key_W))
    {
        renderContext.position += panSpeed * renderContext.direction;
    }
    if(Key(Key_S))
    {
        renderContext.position -= panSpeed * renderContext.direction;
    }
    if(Key(Key_A))
    {
        renderContext.position -= glm::normalize(glm::cross(renderContext.direction, renderContext.up)) * panSpeed;
    }
    if(Key(Key_D))
    {
        renderContext.position += glm::normalize(glm::cross(renderContext.direction, renderContext.up)) * panSpeed;
    }
    
    if(Mouse(MouseLeft))
    {
        glm::vec3 front;
        front.x = (float)(cos(glm::radians(inputState.mousePitch)) * cos(glm::radians(inputState.mouseYaw)));
        front.y = (float)(sin(glm::radians(inputState.mousePitch)));
        front.z = (float)(cos(glm::radians(inputState.mousePitch)) * sin(glm::radians(inputState.mouseYaw)));
        renderContext.direction = glm::normalize(front);
    }
    if(Mouse(MouseRight))
    {
        auto right = glm::normalize(glm::cross(renderContext.direction, renderContext.up));
        renderContext.position += right * panSpeed * (float)-inputState.xDelta;
        auto up = glm::normalize(glm::cross(right, renderContext.direction));
        renderContext.position += glm::normalize(up) * panSpeed * (float)-inputState.yDelta;
    }
    
    renderContext.right = glm::normalize(glm::cross(renderContext.direction, renderContext.up));
    
    renderContext.viewMatrix = glm::lookAt(renderContext.position, renderContext.position + renderContext.direction, renderContext.up);
}

static texture LoadTexture(const char* Path)
{
    texture newTex;
    newTex.data = stbi_load(Path, &newTex.width, &newTex.height, 0, STBI_rgb_alpha);
    
    if(!newTex.data)
    {
        fprintf(stderr, "Could not load texture\n");
        return newTex;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    newTex.textureID = textureID;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newTex.width, newTex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, newTex.data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return newTex;
}



