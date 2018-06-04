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

static void SetFloatUniform(GLuint programID, const char* name, float value)
{
    glUniform1f(glGetUniformLocation(programID, name), value);
}

static void SetBoolUniform(GLuint programID, const char* name, bool val)
{
    glUniform1i(glGetUniformLocation(programID, name), val);
}

struct Resolution
{
    int window_width;
    int window_height;
};

static Resolution get_resolution() 
{
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    
    Resolution res = {};
    res.window_width = mode->width;
    res.window_height = mode->height;
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

static void ComputeMeshTransformation(Mesh& object)
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
            
            auto newLen = fread(source, sizeof(GLchar), (size_t)bufSize, file);
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

static Shader LoadShaders(const char* vertexFilePath, const char* fragmentFilePath)
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
        char* buffer = (char*)malloc((infoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &buffer[0]);
        printf("%s\n", &buffer[0]);
        free(buffer);
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
        free(buffer);
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
        free(buffer);
    }
    
    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    free(vertexShaderCode);
    free(fragmentShaderCode);
    
    Shader newShader = {};
    newShader.programID = programID;
    return newShader;
}

static void InitializeOpenGL(RenderContext& renderContext)
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
    auto res = get_resolution();
    
    renderContext.screenWidth= res.window_width;
    renderContext.screenHeight = res.window_height;
    renderContext.window = glfwCreateWindow(renderContext.screenWidth, renderContext.screenHeight, "Convex Hull", NULL, NULL);
    if(renderContext.window == NULL)
    {
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
    
    GLuint pos = (GLuint)glGetAttribLocation(renderContext.basicShader.programID, "position");
    
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

static void RenderLine(RenderContext& renderContext, glm::vec3 start = glm::vec3(0.0f), glm::vec3 end = glm::vec3(0.0f), glm::vec4 color = glm::vec4(1.0f), float lineWidth = 2.0f)
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

static void CreateDirectionalLight(RenderContext &renderContext, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
    auto &light = renderContext.lights[renderContext.lightCount++];
    light.direction = direction;
    light.ambient = ambient;
    light.diffuse = diffuse;
    light.specular = specular;
    light.lightType = LT_DIRECTIONAL;
}


static void CreateSpotLight(RenderContext &renderContext, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 position, float cutOff, float outerCutOff, float constant, float linear, float quadratic)
{
    auto &light = renderContext.lights[renderContext.lightCount++];
    light.direction = direction;
    light.ambient = ambient;
    light.diffuse = diffuse;
    light.specular = specular;
    
    light.spot.position = position;
    light.spot.cutOff = cutOff;
    light.spot.outerCutOff = outerCutOff;
    light.spot.constant = constant;
    light.spot.linear = linear;
    light.spot.quadratic = quadratic;
    light.lightType = LT_SPOT;
}

static GLfloat* BuildVertexBuffer(Face* faces, size_t numFaces, size_t *vertexCount)
{
    size_t sizeOfArray = 0;
    
    for(size_t i = 0; i < numFaces; i++)
    {
        sizeOfArray += faces[i].vertices.size;
    }
    
    *vertexCount = sizeOfArray;
    
    GLfloat* vertices = (GLfloat*)malloc(sizeOfArray * sizeof(VertexInfo));
    
    for(size_t i = 0; i < numFaces; i++)
    {
        auto currentColor = faces[i].faceColor;
        
        for(size_t j = 0; j < faces[i].vertices.size; j++)
        {
            auto attributes = 10;
            auto baseIndex = i * faces[i].vertices.size * attributes;
            
            auto jNumIndices = j * attributes;
            
            vertices[baseIndex + jNumIndices] = faces[i].vertices[j].position.x;
            vertices[baseIndex + 1 + jNumIndices] = faces[i].vertices[j].position.y;
            vertices[baseIndex + 2 + jNumIndices] = faces[i].vertices[j].position.z;
            
            vertices[baseIndex + 3 + jNumIndices] = faces[i].faceNormal.x;
            vertices[baseIndex + 4 + jNumIndices] = faces[i].faceNormal.y;
            vertices[baseIndex + 5 + jNumIndices] = faces[i].faceNormal.z;
            
            vertices[baseIndex + 6 + jNumIndices] = currentColor.x;
            vertices[baseIndex + 7 + jNumIndices] = currentColor.y;
            vertices[baseIndex + 8 + jNumIndices] = currentColor.z;
            vertices[baseIndex + 9 + jNumIndices] = currentColor.w;
        }
    }
    return vertices;
}

static Mesh& InitEmptyMesh(RenderContext& renderContext, int meshIndex = -1)
{
    if(meshIndex != -1)
    {
        auto& m = renderContext.meshes[meshIndex];
        m.dirty = true;
        return m;
    }
    else
    {
        Mesh& object = renderContext.meshes[renderContext.meshCount++];
        object.meshIndex = renderContext.meshCount - 1;
        glGenVertexArrays(1, &object.VAO);
        glBindVertexArray(object.VAO);
        glGenBuffers(1, &object.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, object.VBO);
        
        auto& mat = object.material;
        mat.specularColor = glm::vec3(1.0f);
        mat.alpha = 1.0f;
        mat.type = MT_color;
        mat.diffuse.color = glm::vec3(1.0f, 1.0f, 1.0f);
        mat.materialShader = renderContext.colorShader;
        
        object.dirty = true;
        
        glBindVertexArray(0);
        
        return object;
    }
}



static glm::vec3 ComputeFaceNormal(Face f)
{
    // Newell's Method
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    glm::vec3 normal = glm::vec3(0.0f);
    
    for(size_t i = 0; i < f.vertices.size; i++)
    {
        auto& current = f.vertices[i].position;
        auto& next = f.vertices[(i + 1) % 3].position;
        
        normal.x = normal.x + (current.y - next.y) * (current.z + next.z);
        normal.y = normal.y + (current.z - next.z) * (current.x + next.x);
        normal.z = normal.z + (current.x - next.x) * (current.y + next.y);
    }
    
    return glm::normalize(normal);
}

static Vertex* LoadObj(const char* filePath, float scale = 1.0f)
{
    Vertex* vertices = nullptr;
    auto file = fopen(filePath, "r");
    if(file)
    {
        size_t vCount = 0;
        char buffer[64];
        
        while(fgets(buffer, 64, file))
        {
            if(startsWith(buffer, "v") && !startsWith(buffer, "vt"))
            {
                vCount++;
            }
        }
        
        vertices = (Vertex*)calloc(vCount, sizeof(Vertex));
        
        rewind(file);
        
        int i = 0;
        
        while(fgets(buffer, 64, file))
        {
            if(startsWith(buffer, "v") && !startsWith(buffer, "vt"))
            {
                sscanf(buffer, "v %f %f %f", &vertices[i].position.x, &vertices[i].position.y, &vertices[i].position.z);
                
                vertices[i].position = vertices[i].position * scale;
                vertices[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
                i++;
            }
        }
        
        fclose(file);
    }
    
    return vertices;
}


static Vertex* LoadObjWithFaces(RenderContext &renderContext, const char* filePath, Mesh &m, int *numberOfPoints, float scale = 1.0f, glm::vec4 color = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f))
{
    m = InitEmptyMesh(renderContext);
    m.dirty = true;
    
    Vertex *vertices = nullptr;
    auto file = fopen(filePath, "r");
    if(file)
    {
        size_t vCount = 0;
        char buffer[64];
        
        while(fgets(buffer, 64, file))
        {
            if(startsWith(buffer, "v") && !startsWith(buffer, "vt"))
            {
                vCount++;
            }
        }
        
        *numberOfPoints = vCount;
        vertices = (Vertex*)calloc(vCount, sizeof(Vertex));
        
        rewind(file);
        
        int i = 0;
        
        bool tangents = false;
        
        while(fgets(buffer, 64, file))
        {
            if(startsWith(buffer, "v") && !startsWith(buffer, "vt"))
            {
                sscanf(buffer, "v %f %f %f", &vertices[i].position.x, &vertices[i].position.y, &vertices[i].position.z);
                
                vertices[i].position = vertices[i].position * scale;
                vertices[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
                i++;
            }
            else if(startsWith(buffer, "vt"))
            {
                tangents = true;
            }
            else if(startsWith(buffer, "f "))
            {
                Face f = {};
                
                char *r = (char*)malloc(strlen(buffer));
                char *str = r;
                strcpy(str, buffer);
                if(tangents)
                {
                    char *tok = strtok(str, " ");
                    int v, t;
                    while ((tok = strtok(nullptr, " ")) != nullptr)
                    {
                        if(!startsWith(tok, "f") && !startsWith(tok, "\n"))
                        {
                            sscanf(tok, "%d/%d", &v, &t);
                            addToList(f.vertices, vertices[v - 1]);
                        }
                    }
                }
                else
                {
                    char *tok = strtok(str, " ");
                    int v;
                    while ((tok = strtok(nullptr, " ")) != nullptr)
                    {
                        if(!startsWith(tok, "f") && !startsWith(tok, "\n"))
                        {
                            sscanf(tok, "%d", &v);
                            addToList(f.vertices, vertices[v - 1]);
                        }
                    }
                }

                free(r);
                f.faceNormal = ComputeFaceNormal(f);
                f.faceColor = color;
                
                m.faces.push_back(f);
            }
        }
        
        fclose(file);
    }
    return vertices;
}


static void RenderPointCloud(RenderContext& renderContext, Vertex* inputPoints, int numPoints)
{
    glBindVertexArray(renderContext.pointCloudVAO);
    glUseProgram(renderContext.particleShader.programID);
    
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.particlesVBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(numPoints * 4 * sizeof(GLfloat)), NULL, GL_STREAM_DRAW);
    
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
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(numPoints * sizeof(GLfloat) * 4), positions);
    
    glBindBuffer(GL_ARRAY_BUFFER, renderContext.particlesColorVBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(numPoints * 4 * sizeof(GLfloat)), NULL, GL_STREAM_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(numPoints * sizeof(GLfloat) * 4), colors);
    
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
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    free(positions);
    free(colors);
}


static void RenderQuad(RenderContext& renderContext, glm::vec3 position = glm::vec3(0.0f), glm::quat orientation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::vec4 color = glm::vec4(1.0f), bool isUI = false)
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
    SetBoolUniform(renderContext.basicShader.programID, "isUI", isUI);
    
    glDrawElements(GL_TRIANGLES, sizeof(renderContext.quadIndices), GL_UNSIGNED_INT, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void RenderFaceEdges(RenderContext& renderContext, std::vector<Edge>& edges, Vertex v1, Vertex v2, Vertex v3, glm::vec4 edgeColor, float lineWidth)
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
        Edge e = {};
        e.origin = v1.vertexIndex;
        e.end = v2.vertexIndex;
        edges.push_back(e);
    }
    
    if(!e2Drawn)
    {
        RenderLine(renderContext, v2.position, v3.position, edgeColor, lineWidth);
        Edge e = {};
        e.origin = v2.vertexIndex;
        e.end = v3.vertexIndex;
        edges.push_back(e);
    }
    
    if(!e3Drawn)
    {
        RenderLine(renderContext, v3.position, v1.position, edgeColor, lineWidth);
        Edge e = {};
        e.origin = v1.vertexIndex;
        e.end = v3.vertexIndex;
        edges.push_back(e);
    }
}

static glm::vec3 ComputeCenterpoint(Face &f)
{
    
    auto center = glm::vec3();
    
    for(size_t i = 0; i < f.vertices.size; i++)
    {
        center += f.vertices[i].position;
    }
    
    return glm::vec3(center.x / f.vertices.size, center.y / f.vertices.size, center.z / f.vertices.size);
}

static void RenderMesh(RenderContext& renderContext, Mesh& m, Vertex *vertices = nullptr)
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
    
    for(int i = 0; i < renderContext.lightCount; i++)
    {
        auto &light = renderContext.lights[i];
        switch(light.lightType)
        {
            case LT_DIRECTIONAL:
            {
                SetVec3Uniform(material.materialShader.programID, "dirLight.direction", light.direction);
                SetVec3Uniform(material.materialShader.programID, "dirLight.ambient", light.ambient);
                SetVec3Uniform(material.materialShader.programID, "dirLight.diffuse", light.diffuse);
                SetVec3Uniform(material.materialShader.programID, "dirLight.specular", light.specular);
            }
            break;
            case LT_SPOT:
            {
                SetVec3Uniform(material.materialShader.programID, "spotLight.position", renderContext.position);
                SetVec3Uniform(material.materialShader.programID, "spotLight.direction", renderContext.direction);
                SetVec3Uniform(material.materialShader.programID, "spotLight.ambient", light.ambient);
                SetVec3Uniform(material.materialShader.programID, "spotLight.diffuse", light.diffuse);
                SetVec3Uniform(material.materialShader.programID, "spotLight.specular", light.specular);
                SetFloatUniform(material.materialShader.programID, "spotLight.cutOff", light.spot.cutOff);
                SetFloatUniform(material.materialShader.programID, "spotLight.outerCutOff", light.spot.outerCutOff);
                SetFloatUniform(material.materialShader.programID, "spotLight.constant", light.spot.constant);
                SetFloatUniform(material.materialShader.programID, "spotLight.linear", light.spot.linear);
                SetFloatUniform(material.materialShader.programID, "spotLight.quadratic", light.spot.quadratic);
            }
            break;
        }
    }
    
    SetVec3Uniform(material.materialShader.programID, "viewPos", renderContext.position);
    
    if(material.type == MT_color)
    {
        SetVec3Uniform(material.materialShader.programID, "diffuseColor", material.diffuse.color);
    }
    
    glBindVertexArray(m.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    
    if(m.dirty)
    {
        free(m.currentVBO);
        m.dirty = false;
        m.currentVBO = BuildVertexBuffer(m.faces.data(), m.faces.size(), &m.vertexCount);
    }
    
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(VertexInfo) * m.vertexCount), &m.currentVBO[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(Vertex, position));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*)offsetof(Vertex, normal));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void*) offsetof(Vertex, color));
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m.vertexCount * 10);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    
    if(vertices)
    {
        RenderLine(renderContext, vertices[renderContext.nonConvexEdge.origin].position, vertices[renderContext.nonConvexEdge.end].position, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 5.0f);
    }
}

static void RenderGrid(RenderContext& renderContext, glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), float lineWidth = 2.0f)
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


static void Render(RenderContext& renderContext)
{
    for(int meshIndex = 0; meshIndex < renderContext.meshCount; meshIndex++)
    {
        RenderMesh(renderContext, renderContext.meshes[meshIndex]);
    }
    //RenderGrid(renderContext, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 1.0f);
}

static void ComputeMatrices(RenderContext& renderContext, double deltaTime)
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



