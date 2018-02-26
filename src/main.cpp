#include <ctime>
#include <random>
#include <vector>
#include <cstdio>
#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util.h"
#include "keys.h"

const static float globalScale = 0.2f;

static input_state inputState;

#include "keys.cpp"

#include "rendering.h"
#include "rendering.cpp"
#include "quickhull.h"

#define Min(A,B) ((A < B) ? (A) : (B))
#define Max(A,B) ((A > B) ? (A) : (B))
#define Abs(x) ((x) < 0 ? -(x) : (x))

static vertex* GeneratePoints(render_context& renderContext, int numberOfPoints, float min = 0.0f, float max = 200.0f)
{
    auto res = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++)
    {
        float x = RandomFloat(min, max);
        float y = RandomFloat(min, max);
        float z = RandomFloat(min, max);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = RandomColor(); //glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        res[i].numFaceHandles = 0;
        res[i].vertexIndex = i;
    }
    return res;
}

int main()
{
    srand(time(NULL));
    render_context renderContext = {};
    renderContext.FoV = 45.0f;
    renderContext.position = glm::vec3(0.0f, 50.5f, 30.0f);
    renderContext.direction = glm::vec3(0.0f, -0.75f, -1.0f);
    renderContext.up = glm::vec3(0.0f, 1.0f, 0.0f);
    renderContext.near = 0.1f;
    renderContext.far =  10000.0f;
    renderContext.originOffset = glm::vec3(5.0f, 0.0f, 5.0f);
    renderContext.renderPoints = true;
    renderContext.renderNormals = true;
    
    InitializeOpenGL(renderContext);
    
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    inputState.mouseYaw = -90.0f;
    inputState.mousePitch = -45.0f;
    
    CreateLight(renderContext, glm::vec3(0.0f, 75.0f, 10.0f), glm::vec3(1, 1, 1), 2500.0f);
    
    int numberOfPoints = 20;
    
    auto vertices = GeneratePoints(renderContext, numberOfPoints, 0.0f, 100.0f);
    
    auto disableMouse = false;
    
    stack faceStack;
    auto& m = InitQuickHull(renderContext, vertices, numberOfPoints, faceStack);
    
    QHIteration nextIter = QHIteration::findNextIter;
    face* currentFace = nullptr;
    std::vector<face> v;
    int previousIteration = 0;
    
    // Check if the ESC key was pressed or the window was closed
    while(!KeyDown(Key_Escape) &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime);
        
        if(KeyDown(Key_Q))
        {
            QuickHull(renderContext, vertices, numberOfPoints, m, faceStack);
        }
        
        if(KeyDown(Key_J))
        {
            switch(nextIter)
            {
                case QHIteration::findNextIter:
                {
                    printf("Finding next iteration\n");
                    currentFace = QuickHullFindNextIteration(renderContext, m, vertices, faceStack);
                    nextIter = QHIteration::findHorizon;
                }
                break;
                case QHIteration::findHorizon:
                {
                    if(currentFace)
                    {
                        printf("Finding horizon\n");
                        QuickHullHorizon(renderContext, m, vertices, *currentFace, v, faceStack,  &previousIteration);
                        nextIter = QHIteration::doIter;
                    }
                    
                }
                break;
                case QHIteration::doIter:
                {
                    if(currentFace)
                    {
                        printf("Doing iteration\n");
                        QuickHullIteration(renderContext, m, vertices, faceStack, *currentFace, v, previousIteration);
                        nextIter = QHIteration::findNextIter;
                        v.clear();
                    }
                }
                break;
            }
            
        }
        
        if(KeyDown(Key_P))
        {
            renderContext.renderPoints = !renderContext.renderPoints;
        }
        if(KeyDown(Key_N))
        {
            renderContext.renderNormals = !renderContext.renderNormals;
        }
        
        if(renderContext.renderPoints)
        {
            RenderPointCloud(renderContext, vertices, numberOfPoints);
        }
        
        Render(renderContext, vertices);
        
        if(KeyDown(Key_9))
        {
            disableMouse = !disableMouse;
            if(disableMouse)
            {
                glfwSetInputMode(renderContext.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else
            {
                glfwSetInputMode(renderContext.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            }
        }
        
        // Swap buffers
        glfwSwapBuffers(renderContext.window);
        
        inputState.xScroll = 0;
        inputState.yScroll = 0;
        inputState.xDelta = 0;
        inputState.yDelta = 0;
        SetInvalidKeys();
        
        glfwPollEvents();
    }
    
    return 0;
}

