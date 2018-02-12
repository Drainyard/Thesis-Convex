
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

#define Min(A,B) ((A < B) ? (A) : (B))
#define Max(A,B) ((A > B) ? (A) : (B))
#define Abs(x) ((x) < 0 ? -(x) : (x))

static model InitializeCube(render_context& renderContext)
{
    model cube = {};
    
    glGenVertexArrays(1, &cube.VAO);
    
    glBindVertexArray(cube.VAO);
    
    glGenBuffers(1, &cube.VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    
    glGenBuffers(1, &cube.uvBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, cube.uvBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    
    
    cube.position = glm::vec3(0, 0, 0);
    cube.orientation = glm::vec3(0, 0, 0);
    cube.transform = glm::mat4(1.0f);
    
    texture tex = LoadTGA("../assets/textures/uvtemplate.tga");
    cube.modelTexture = tex;
    
    return cube;
}

int main()
{
    render_context renderContext = {};
    renderContext.position = glm::vec3(4, 3, 3);
    renderContext.FoV = 45.0f;
    
    InitializeOpenGL(renderContext);
    auto cube = InitializeCube(renderContext);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    
    // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(renderContext.window, Key_Escape ) != GLFW_PRESS &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime, glm::vec3(0, 0, 0));
        RenderBasicCube(renderContext, cube, deltaTime);
        
        // Swap buffers
        glfwSwapBuffers(renderContext.window);
        
        inputState.xScroll = 0;
        inputState.yScroll = 0;
        inputState.xDelta = 0;
        inputState.yDelta = 0;
        
        glfwPollEvents();
    }
    
    return 0;
}

