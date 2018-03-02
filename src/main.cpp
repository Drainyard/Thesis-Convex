#include <ctime>
#include <random>
#include <vector>
#include <stack>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
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
#include "naive.h"

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
    srand((unsigned int)time(NULL));
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
    renderContext.faceCounter = 0;
    
    InitializeOpenGL(renderContext);
    
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    inputState.mouseYaw = -90.0f;
    inputState.mousePitch = -45.0f;
    
    CreateLight(renderContext, glm::vec3(0.0f, 75.0f, 10.0f), glm::vec3(1, 1, 1), 2500.0f);
    
    int numberOfPoints = 10;
    
    auto vertices = GeneratePoints(renderContext, numberOfPoints, 0.0f, 100.0f);
    
    auto naiveVertices = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    memcpy(naiveVertices, vertices, sizeof(vertex) * numberOfPoints);
    
    auto quickHullVertices = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    memcpy(quickHullVertices, vertices, sizeof(vertex) * numberOfPoints);
    
    auto finalQHVertices = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    memcpy(finalQHVertices, vertices, sizeof(vertex) * numberOfPoints);
    
    auto disableMouse = false;
    
    std::stack<int> faceStack;
    auto& mq = InitQuickHull(renderContext, quickHullVertices, numberOfPoints, faceStack);
    mesh* mFinal = nullptr;
    
    QHIteration nextIter = QHIteration::findNextIter;
    face* currentFace = nullptr;
    std::vector<int> v;
    int previousIteration = 0;
    
    auto& mn = NaiveConvexHull(renderContext, naiveVertices, numberOfPoints);
    //auto& mn = InitEmptyMesh(renderContext);
    
    mesh* currentMesh = &mq;
    
    // Check if the ESC key was pressed or the window was closed
    while(!KeyDown(Key_Escape) &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime);
        
        if(KeyDown(Key_H))
        {
            if(!mFinal)
            {
                mFinal = &QuickHull(renderContext, finalQHVertices, numberOfPoints);
            }
            if(mFinal)
            {
                currentMesh = mFinal;
            }
        }
        
        if(KeyDown(Key_J))
        {
            if(faceStack.size() > 0)
            {
                switch(nextIter)
                {
                    case QHIteration::findNextIter:
                    {
                        printf("Finding next iteration\n");
                        currentFace = QuickHullFindNextIteration(renderContext, mq, quickHullVertices, faceStack);
                        if(currentFace)
                        {
                            nextIter = QHIteration::findHorizon;
                        }
                    }
                    break;
                    case QHIteration::findHorizon:
                    {
                        if(currentFace)
                        {
                            printf("Finding horizon\n");
                            QuickHullHorizon(renderContext, mq, quickHullVertices, *currentFace, v, &previousIteration);
                            nextIter = QHIteration::doIter;
                        }
                    }
                    break;
                    case QHIteration::doIter:
                    {
                        if(currentFace)
                        {
                            printf("Doing iteration\n");
                            QuickHullIteration(renderContext, mq, quickHullVertices, faceStack, *currentFace, v, previousIteration, numberOfPoints);
                            nextIter = QHIteration::findNextIter;
                            v.clear();
                        }
                    }
                    break;
                }
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
        
        if(KeyDown(Key_Q))
        {
            currentMesh = &mq;
        }
        
        if(KeyDown(Key_B))
        {
            currentMesh = &mn;
        }
        
        if(currentMesh)
        {
            RenderMesh(renderContext, *currentMesh, vertices);
        }
        
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

