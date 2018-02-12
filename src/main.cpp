
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include "keys.h"

static void PrintError()
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
        printf("Bob: %s\n", &Buffer[0]);
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
    
    return ProgramID;
}

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
    window = glfwCreateWindow( 1024, 768, "Tutorial 01", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    printf("%s\n", glGetString(GL_VERSION));
    printf("Shading language supported: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Glad Version: %d.%d\n", GLVersion.major, GLVersion.minor);
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    
    printf("VAO: %d\n", VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };
    
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    GLuint ProgramID = LoadShaders("../shaders/basic.vert", "../shaders/basic.frag");
    
    // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(window, Key_Escape ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnableVertexAttribArray(0);
        glUseProgram(ProgramID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    
    return 0;
}
