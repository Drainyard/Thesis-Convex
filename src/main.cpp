
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include "keys.h"

static void PrintError()
{
    if(auto err = glGetError() != GL_NO_ERROR)
        printf("%d\n", glGetError());
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
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };
    
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    PrintError();
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    PrintError();
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    PrintError();
    // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(window, Key_Escape ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 )
    {
        glEnableVertexAttribArray(0);
        PrintError();
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        PrintError();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        PrintError();
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        PrintError();
        glDisableVertexAttribArray(0);
        PrintError();
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }
    
    return 0;
}
