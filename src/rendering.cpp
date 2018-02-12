
static void SetMatrixUniform(GLuint programID, const char* name, glm::mat4 matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, &matrix[0][0]);
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
    auto width = 1024;
    auto height = 768;
    renderContext.window = glfwCreateWindow(width, height, "Convex Hull", NULL, NULL);
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
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    
    glfwSetInputMode(renderContext.window, GLFW_STICKY_KEYS, GL_TRUE);
    
    renderContext.basicShader = LoadShaders("../shaders/basic.vert", "../shaders/basic.frag");
}


static void ComputeModelTransformation(render_context& renderContext , double deltaTime, model& object)
{
    if(glfwGetMouseButton(renderContext.window, Key_MouseLeft) == GLFW_PRESS)
    {
        object.orientation = glm::vec3(object.orientation.x - (float)inputState.yDelta * (float)deltaTime, object.orientation.y - (float)inputState.xDelta * (float)deltaTime, 0);
        glm::quat qY = glm::quat(object.orientation.y - (float)inputState.yDelta * (float)deltaTime, 0, 1, 0);
        glm::quat qX = glm::quat(object.orientation.x - (float)inputState.xDelta * (float)deltaTime, 1, 0 ,1);
        auto rot = qY * qX;
        object.transform = glm::toMat4(rot);
        object.transform = glm::translate(object.transform, glm::vec3(0, 0, 0));
        
    }
}

static void RenderBasicCube(render_context& renderContext, model& cube, double deltaTime)
{
    glUseProgram(renderContext.basicShader.programID);
    ComputeModelTransformation(renderContext, deltaTime, cube);
    SetMatrixUniform(renderContext.basicShader.programID, "M", cube.transform);
    SetMatrixUniform(renderContext.basicShader.programID, "V", renderContext.viewMatrix);
    SetMatrixUniform(renderContext.basicShader.programID, "P", renderContext.projectionMatrix);
    
    glBindVertexArray(cube.VAO);
    
    glBindTexture(GL_TEXTURE_2D, cube.modelTexture.textureID);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, cube.uvBufferHandle);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
    glDisableVertexAttribArray(0);
    
}

static void Render(render_context& renderContext, double deltaTime)
{
    
}

static void ComputeMatrices(render_context& renderContext, double deltaTime, glm::vec3 target)
{
    
    renderContext.projectionMatrix = glm::perspective(glm::radians(renderContext.FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    
    auto scrollSpeed = 10.0f;
    
    renderContext.position += glm::vec3(target.x - renderContext.position.x * deltaTime * inputState.yScroll * scrollSpeed, target.y - renderContext.position.y * deltaTime * inputState.yScroll * scrollSpeed, target.z - renderContext.position.z * deltaTime * inputState.yScroll * scrollSpeed);
    renderContext.viewMatrix = glm::lookAt(renderContext.position, target, glm::vec3(0, 1, 0));
}


static texture LoadTGA(const char* Path)
{
    texture newTex;
    newTex.data = stbi_load(Path, &newTex.width, &newTex.height, 0, STBI_rgb);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    newTex.textureID = textureID;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newTex.width, newTex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, newTex.data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return newTex;
}



