
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>


#include "keys.h"

static input_state inputState;

#include "keys.cpp"



#include "rendering.h"
#include "rendering.cpp"

static void PrintError()
{
    if(auto err = glGetError() != GL_NO_ERROR)
        printf("%d\n", err);
}

static GLuint LoadBMP(const char* Path)
{
    unsigned char Header[54];
    unsigned int DataPos;
    unsigned int Width, Height;
    unsigned int ImageSize;
    
    unsigned char* Data;
    
    FILE* File = fopen(Path, "rb");
    if(!File)
    {
        fprintf(stderr, "Image could not be opened: %s\n", Path);
    }
    
    if(fread(Header, 1, 54, File) != 54)
    {
        fprintf(stderr, "Not correct BMP format\n");
        return false;
    }
    
    if(Header[0] != 'B' || Header[1] != 'M')
    {
        fprintf(stderr, "Not correct BMP format\n");
        return false;
    }
    
    DataPos = *(int*)&Header[0x0A];
    ImageSize = *(int*)&Header[0x22];
    Width = *(int*)&Header[0x12];
    Height = *(int*)&Header[0x16];
    
    if(ImageSize == 0)
    {
        ImageSize = Width * Height * 3; //RGB
    }
    
    if(DataPos == 0)
    {
        DataPos = 54;
    }
    
    Data = (unsigned char*)malloc(ImageSize * sizeof(unsigned char));
    fread(Data, 1, ImageSize, File);
    fclose(File);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, Width, Height, 0, GL_BGR, GL_UNSIGNED_BYTE, Data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return textureID;
}

struct texture
{
    int TextureID;
    int Width;
    int Height;
    unsigned char* Data;
};

static texture LoadTGA(const char* Path)
{
    texture Texture;
    Texture.Data = stbi_load(Path, &Texture.Width, &Texture.Height, 0, STBI_rgb);
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    Texture.TextureID = textureID;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture.Width, Texture.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture.Data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    return Texture;
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

static 

static GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    auto VertexShaderCode = LoadShaderFromFile(vertex_file_path);
    auto FragmentShaderCode = LoadShaderFromFile(fragment_file_path);
    if(!VertexShaderCode)
    {
        fprintf(stderr, "Could not load vertex shader: %s\n", vertex_file_path);
    }
    if(!FragmentShaderCode)
    {
        fprintf(stderr, "Could not load fragment shader: %s\n", fragment_file_path);
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    glShaderSource(VertexShaderID, 1, &VertexShaderCode, NULL);
    glCompileShader(VertexShaderID);
    
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if(InfoLogLength > 0)
    {
        char* Buffer = (char*)malloc((InfoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &Buffer[0]);
        printf("%s\n", &Buffer[0]);
    }
    
    
    glShaderSource(FragmentShaderID, 1, &FragmentShaderCode, NULL);
    glCompileShader(FragmentShaderID);
    
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if(InfoLogLength > 0)
    {
        char* Buffer = (char*)malloc((InfoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &Buffer[0]);
        printf("%s\n", &Buffer[0]);
    }
    
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if(InfoLogLength > 0)
    {
        char* Buffer = (char*)malloc((InfoLogLength + 1) * sizeof(char));
        glGetShaderInfoLog(ProgramID, InfoLogLength, NULL, &Buffer[0]);
        printf("Bob: %s\n", &Buffer[0]);
    }
    
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    free(VertexShaderCode);
    free(FragmentShaderCode);
    
    return ProgramID;
}


static void SetMatrixUniform(GLuint ProgramID, const char* Name, glm::mat4 Matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(ProgramID, Name), 1, GL_FALSE, &Matrix[0][0]);
}


static const GLfloat g_vertex_buffer_data[] = {
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f,-1.0f, // triangle 2 : begin
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, // triangle 2 : end
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

static const GLfloat g_color_buffer_data[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
};

static const GLfloat g_uv_buffer_data[] = {
    0.000059f, 1.0f-0.000004f,
    0.000103f, 1.0f-0.336048f,
    0.335973f, 1.0f-0.335903f,
    1.000023f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.336024f, 1.0f-0.671877f,
    0.667969f, 1.0f-0.671889f,
    1.000023f, 1.0f-0.000013f,
    0.668104f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.000059f, 1.0f-0.000004f,
    0.335973f, 1.0f-0.335903f,
    0.336098f, 1.0f-0.000071f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.336024f, 1.0f-0.671877f,
    1.000004f, 1.0f-0.671847f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.668104f, 1.0f-0.000013f,
    0.335973f, 1.0f-0.335903f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.668104f, 1.0f-0.000013f,
    0.336098f, 1.0f-0.000071f,
    0.000103f, 1.0f-0.336048f,
    0.000004f, 1.0f-0.671870f,
    0.336024f, 1.0f-0.671877f,
    0.000103f, 1.0f-0.336048f,
    0.336024f, 1.0f-0.671877f,
    0.335973f, 1.0f-0.335903f,
    0.667969f, 1.0f-0.671889f,
    1.000004f, 1.0f-0.671847f,
    0.667979f, 1.0f-0.335851f
};

#define Min(A,B) ((A < B) ? (A) : (B))
#define Max(A,B) ((A > B) ? (A) : (B))
#define Abs(x) ((x) < 0 ? -(x) : (x))


int main()
{
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 
    
    // Open a window and create its OpenGL context
    GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
    auto width = 1024;
    auto height = 768;
    window = glfwCreateWindow( 1024, 768, "Tutorial 01", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    glfwSetCursorPosCallback(window, MousePositionCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);
    
    printf("%s\n", glGetString(GL_VERSION));
    printf("Shading language supported: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Glad Version: %d.%d\n", GLVersion.major, GLVersion.minor);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    
    printf("VAO: %d\n", VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    
    glm::mat4 Model = glm::mat4(1.0f);
    
    model cube = {};
    
    cube.position = glm::vec3(0, 0, 0);
    cube.orientation = glm::vec3(0, 0, 0);
    cube.transform = glm::mat4(1.0f);
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    GLuint ProgramID = LoadShaders("../shaders/basic.vert", "../shaders/basic.frag");
    
    texture Texture = LoadTGA("../assets/textures/uvtemplate.tga");
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    
    render_context renderer = {};
    renderer.position = glm::vec3(4, 3, 3);
    renderer.horizontalAngle = 3.14f;
    renderer.verticalAngle = 0.0f;
    renderer.FoV = 45.0f;
    renderer.speed = 3.0f;
    renderer.mouseSpeed = 0.005f;
    renderer.window = window;
    
    // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(window, Key_Escape ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(ProgramID);
        
        
        ComputeMatrices(renderer, deltaTime, glm::vec3(0, 0, 0));
        ComputeModelTransformation(renderer, deltaTime, cube);
        SetMatrixUniform(ProgramID, "M", cube.transform);
        SetMatrixUniform(ProgramID, "V", renderer.viewMatrix);
        SetMatrixUniform(ProgramID, "P", renderer.projectionMatrix);
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
        glDisableVertexAttribArray(0);
        
        // Swap buffers
        glfwSwapBuffers(window);
        inputState.xScroll = 0;
        inputState.yScroll = 0;
        inputState.xDelta = 0;
        inputState.yDelta = 0;
        glfwPollEvents();
        
    }
    
    return 0;
}

