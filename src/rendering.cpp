static void SetMatrixUniform(GLuint programID, const char* name, glm::mat4 matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, &matrix[0][0]);
}

static void SetVec3Uniform(GLuint programID, const char* name, glm::vec3 vec)
{
    glUniform3f(glGetUniformLocation(programID, name), vec.x, vec.y, vec.z);
}

static void SetFloatUniform(GLuint programID, const char* name, float value)
{
    glUniform1f(glGetUniformLocation(programID, name), value);
}


static void PrintGLError()
{
    if(auto err = glGetError() != GL_NO_ERROR)
        printf("%d\n", err);
}


static char* LoadShaderFromFile(const char* Path)
{
    GLchar* Source = {};
    
    FILE* File = fopen(Path, "r");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        uint32_t Size = (uint32_t)ftell(File);
        fseek(File, 0, SEEK_SET);
        
        Source = (GLchar*)malloc(Size + 1 * sizeof(GLchar));
        
        fread(Source, Size, 1, File);
        Source[Size] = '\0';
        
        fclose(File);
    }
    else
    {
        fprintf(stderr, "Could not read file %s. File does not exist.\n",Path);
    }
    
    return Source;
}

static shader LoadShaders(const char* vertexFilePath, const char* fragmentFilePath)
{
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    auto VertexShaderCode = LoadShaderFromFile(vertexFilePath);
    auto FragmentShaderCode = LoadShaderFromFile(fragmentFilePath);
    if(!VertexShaderCode)
    {
        fprintf(stderr, "Could not load vertex shader: %s\n", vertexFilePath);
    }
    if(!FragmentShaderCode)
    {
        fprintf(stderr, "Could not load fragment shader: %s\n", fragmentFilePath);
    }
    
    GLint result = GL_FALSE;
    int infoLogLength;
    
    glShaderSource(vertexShaderID, 1, &VertexShaderCode, NULL);
    glCompileShader(vertexShaderID);
    
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0)
    {
        char* Buffer = (char*)malloc((infoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &Buffer[0]);
        printf("%s\n", &Buffer[0]);
    }
    
    
    glShaderSource(fragmentShaderID, 1, &FragmentShaderCode, NULL);
    glCompileShader(fragmentShaderID);
    
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0)
    {
        char* Buffer = (char*)malloc((infoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &Buffer[0]);
        printf("%s\n", &Buffer[0]);
    }
    
    printf("Linking program\n");
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);
    
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if(infoLogLength > 0)
    {
        char* Buffer = (char*)malloc((infoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(programID, infoLogLength, NULL, &Buffer[0]);
        printf("Bob: %s\n", &Buffer[0]);
    }
    
    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    free(VertexShaderCode);
    free(FragmentShaderCode);
    
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
    renderContext.screenWidth= 1024;
    renderContext.screenHeight = 768;
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
    
    printf("%s\n", glGetString(GL_VERSION));
    printf("Shading language supported: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Glad Version: %d.%d\n", GLVersion.major, GLVersion.minor);
    
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glfwSetInputMode(renderContext.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    renderContext.basicShader = LoadShaders("../shaders/basic.vert", "../shaders/basic.frag");
    
}


static void ComputeModelTransformation(render_context& renderContext , double deltaTime, model& object)
{
    if(glfwGetMouseButton(renderContext.window, Key_MouseLeft) == GLFW_PRESS)
    {
        
    }
}

static model LoadModel(render_context& renderContext, gl_buffer vbo, gl_buffer* uvBuffer = 0, gl_buffer* colorBuffer = 0, gl_buffer* normalBuffer = 0)
{
    model object = {};
    
    glGenVertexArrays(1, &object.VAO);
    glBindVertexArray(object.VAO);
    glGenBuffers(1, &object.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, object.VBO);
    glBufferData(GL_ARRAY_BUFFER, vbo.size, vbo.data, GL_STATIC_DRAW);
    
    if(uvBuffer)
    {
        glGenBuffers(1, &object.uvBufferHandle);
        glBindBuffer(GL_ARRAY_BUFFER, object.uvBufferHandle);
        glBufferData(GL_ARRAY_BUFFER, uvBuffer->size, uvBuffer->data, GL_STATIC_DRAW);
        object.hasUV = true;
        object.modelTexture = uvBuffer->uv.tex;
    }
    
    if(colorBuffer)
    {
        glGenBuffers(1, &object.colorBufferHandle);
        glBindBuffer(GL_ARRAY_BUFFER, object.colorBufferHandle);
        glBufferData(GL_ARRAY_BUFFER, colorBuffer->size, colorBuffer->data, GL_STATIC_DRAW);
        object.hasColor = true;
    }
    
    if(normalBuffer)
    {
        glGenBuffers(1, &object.normalBufferHandle);
        glBindBuffer(GL_ARRAY_BUFFER, object.normalBufferHandle);
        glBufferData(GL_ARRAY_BUFFER, normalBuffer->size, normalBuffer->data, GL_STATIC_DRAW);
        object.hasNormals = true;
    }
    
    
    object.position = glm::vec3(0);
    object.orientation = glm::vec3(0);
    object.transform = glm::mat4(1.0f);
    
    return object;
}

static void RenderModel(render_context& renderContext, model& m, double deltaTime, float alpha = 1.0, light* l = 0)
{
    glUseProgram(renderContext.basicShader.programID);
    ComputeModelTransformation(renderContext, deltaTime, m);
    SetMatrixUniform(renderContext.basicShader.programID, "M", m.transform);
    SetMatrixUniform(renderContext.basicShader.programID, "V", renderContext.viewMatrix);
    SetMatrixUniform(renderContext.basicShader.programID, "P", renderContext.projectionMatrix);
    
    SetVec3Uniform(renderContext.basicShader.programID, "lightPosWorld", l ? l->position : glm::vec3(1, 1, 1));
    SetVec3Uniform(renderContext.basicShader.programID, "lightColor", l ? l->color : glm::vec3(1, 1, 1));
    SetFloatUniform(renderContext.basicShader.programID, "lightPower", l ? l->power : 2.0f);
    
    SetFloatUniform(renderContext.basicShader.programID, "alpha", alpha);
    
    glBindVertexArray(m.VAO);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    if(m.hasUV)
    {
        glBindTexture(GL_TEXTURE_2D, m.modelTexture.textureID);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, m.uvBufferHandle);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    if(m.hasNormals)
    {
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, m.normalBufferHandle);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
    glDisableVertexAttribArray(0);
}

static void Render(render_context& renderContext, double deltaTime)
{
    
}

static void ComputeMatrices(render_context& renderContext, double deltaTime, glm::vec3 target)
{
    
    
    if(renderContext.FoV >= 1.0f && renderContext.FoV <= 45.0f)
        renderContext.FoV -= inputState.yScroll;
    if(renderContext.FoV <= 1.0f)
        renderContext.FoV = 1.0f;
    if(renderContext.FoV >= 45.0f)
        renderContext.FoV = 45.0f;
    
    renderContext.projectionMatrix = glm::perspective(glm::radians(renderContext.FoV), (float)renderContext.screenWidth / (float)renderContext.screenHeight, 0.1f, 100.0f);
    
    auto scrollSpeed = 10.0f;
    
    
    float cameraSpeed = 2.5f * deltaTime;
    float panSpeed = 5.0f * deltaTime;
    if(glfwGetKey(renderContext.window, Key_W) == GLFW_PRESS)
    {
        renderContext.position += cameraSpeed * renderContext.direction;
    }
    if(glfwGetKey(renderContext.window, Key_S) == GLFW_PRESS)
    {
        renderContext.position -= cameraSpeed * renderContext.direction;
    }
    if(glfwGetKey(renderContext.window, Key_A) == GLFW_PRESS)
    {
        renderContext.position -= glm::normalize(glm::cross(renderContext.direction, renderContext.up)) * cameraSpeed;
    }
    if(glfwGetKey(renderContext.window, Key_D) == GLFW_PRESS)
    {
        renderContext.position += glm::normalize(glm::cross(renderContext.direction, renderContext.up)) * cameraSpeed;
    }
    
    if(glfwGetMouseButton(renderContext.window, Key_MouseLeft) == GLFW_PRESS)
    {
        glm::vec3 front;
        front.x = cos(glm::radians(inputState.mousePitch)) * cos(glm::radians(inputState.mouseYaw));
        front.y = sin(glm::radians(inputState.mousePitch));
        front.z = cos(glm::radians(inputState.mousePitch)) * sin(glm::radians(inputState.mouseYaw));
        renderContext.direction = glm::normalize(front);
    }
    if(glfwGetMouseButton(renderContext.window, Key_MouseRight) == GLFW_PRESS)
    {
        auto ySign = inputState.yDelta < 0 ? -1.0f : 1.0f;
        renderContext.position += glm::normalize(glm::cross(renderContext.direction, renderContext.up)) * panSpeed * (float)-inputState.xDelta;
        renderContext.position += glm::normalize(renderContext.up) * panSpeed * (float)-inputState.yDelta;
        
    }
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



