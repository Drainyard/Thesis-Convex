
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

int main()
{
    render_context renderContext = {};
    renderContext.FoV = 45.0f;
    renderContext.position = glm::vec3(-10.0f, 3.0f, 3.0f);
    renderContext.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    renderContext.up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    InitializeOpenGL(renderContext);
    
    gl_buffer vbo = {};
    vbo.type = BT_vertexBuffer;
    vbo.data = vertices;
    vbo.size = sizeof(vertices);
    
    gl_buffer uvBuffer = {};
    uvBuffer.type = BT_uvBuffer;
    uvBuffer.data = uvs;
    uvBuffer.size = sizeof(uvs);
    uvBuffer.uv.tex = LoadTexture("../assets/textures/container.jpg");
    
    gl_buffer normalBuffer = {};
    normalBuffer.type = BT_normalBuffer;
    normalBuffer.data = normals;
    normalBuffer.size = sizeof(normals);
    
    auto cube = LoadModel(renderContext, vbo, &uvBuffer, 0, &normalBuffer);
    
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    
    light l = {};
    l.position = glm::vec3(1, 1, 1);
    l.color = glm::vec3(1, 1, 1);
    l.power = 2.0f;
    
    // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(renderContext.window, Key_Escape ) != GLFW_PRESS &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime, glm::vec3(0, 0, 0));
        
        RenderModel(renderContext, cube, deltaTime, 0.3f, &l);
        
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

