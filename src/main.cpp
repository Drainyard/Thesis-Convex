
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
    renderContext.position = glm::vec3(0.0f, 7.5f, 10.0f);
    renderContext.direction = glm::vec3(0.0f, -0.75f, -1.0f);
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
    
    auto& m1 = LoadModel(renderContext, vbo, 0, 0, &normalBuffer, glm::vec3(0.5f, 0.5f, 0.0f));
    auto& m2 = LoadModel(renderContext, vbo, 0, 0, &normalBuffer, glm::vec3(0.5f, 0.5f, 0.0f));
    
    m1.position = glm::vec3(0, 0, 0);
    m1.material.diffuse.diffuseColor = glm::vec3(1.0, 0.0, 0.0);
    m1.scale = glm::vec3(5.0f, 1.0f, 1.0f);
    m2.position = glm::vec3(0, 2, 0);
    m2.material.diffuse.diffuseColor = glm::vec3(0.0, 1.0, 0.0);
    m2.scale = glm::vec3(5.0f, 3.0f, 3.0f);
    m2.orientation = glm::normalize(glm::quat(glm::radians(23.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    inputState.mouseYaw = -90.0f;
    inputState.mousePitch = -45.0f;
    
    CreateLight(renderContext, glm::vec3(1, 1, 1), glm::vec3(1, 1, 1), 2.0f);
    
    // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(renderContext.window, Key_Escape ) != GLFW_PRESS &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime, glm::vec3(0, 0, 0));
        
        Render(renderContext, deltaTime);
        
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

