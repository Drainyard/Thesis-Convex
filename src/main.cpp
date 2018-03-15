#include <ctime>
#include <random>
#include <vector>
#include <map>
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

static vertex* GeneratePoints(render_context& renderContext, int numberOfPoints, coord_t min = 0.0f, coord_t max = 200.0f)
{
    auto res = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++)
    {
        coord_t x = RandomCoord(min, max);
        coord_t y = RandomCoord(min, max);
        coord_t z = RandomCoord(min, max);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        res[i].numFaceHandles = 0;
        res[i].vertexIndex = i;
        res[i].faceHandles = (int*)malloc(sizeof(int) * 2048);
    }
    return res;
}

static vertex* GeneratePointsOnSphere(render_context& renderContext, int numberOfPoints, coord_t radius)
{
    auto res = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * RandomCoord(0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * RandomCoord(0.0, 1.0));
        coord_t x = (coord_t)sin(phi) * (coord_t)cos(theta) * radius;
        coord_t y = (coord_t)sin(phi) * (coord_t)sin(theta) * radius;
        coord_t z = (coord_t)cos(phi) * radius;
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        res[i].numFaceHandles = 0;
        res[i].vertexIndex = i;
        res[i].faceHandles = (int*)malloc(sizeof(int) * 1024);
    }
    return res;
}


static vertex* GeneratePointsInSphere(render_context& renderContext, int numberOfPoints, coord_t min = 0.0f, coord_t max = 200.0f)
{
    auto res = (vertex*)malloc(sizeof(vertex) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++) 
    {
        coord_t theta = 2 * (coord_t)M_PI * RandomCoord(0.0, 1.0);
        coord_t phi = (coord_t)acos(1 - 2 * RandomCoord(0.0, 1.0));
        
        auto r = RandomCoord(min, max);
        coord_t x = r * (coord_t)sin(phi) * (coord_t)cos(theta);
        coord_t y = r * (coord_t)sin(phi) * (coord_t)sin(theta);
        coord_t z = r * (coord_t)cos(phi);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        res[i].numFaceHandles = 0;
        res[i].vertexIndex = i;
        res[i].faceHandles = (int*)malloc(sizeof(int) * 1024);
    }
    return res;
}


vertex* GenerateNewPointSet(render_context& renderContext, vertex** naive, vertex** quickhull, vertex** user, int numVertices, coord_t rangeMin, coord_t rangeMax)
{
    auto vertices = GeneratePointsInSphere(renderContext, numVertices, rangeMin, rangeMax);
    *naive = CopyVertices(vertices, numVertices);
    *quickhull = CopyVertices(vertices, numVertices);
    *user = CopyVertices(vertices, numVertices);
    return vertices;
}

int main()
{
    // Degenerate: 1520515408
    //auto seed = 1520515408; //1520254626;//time(NULL);
    auto seed = time(NULL);
    srand((unsigned int)seed);
    printf("Seed: %ld\n", seed);
    render_context renderContext = {};
    renderContext.FoV = 45.0f;
    renderContext.position = glm::vec3(0.0f, 50.5f, 30.0f);
    renderContext.direction = glm::vec3(0.0f, -0.75f, -1.0f);
    renderContext.up = glm::vec3(0.0f, 1.0f, 0.0f);
    renderContext.near = 0.1f;
    renderContext.far =  10000.0f;
    renderContext.originOffset = glm::vec3(5.0f, 0.0f, 5.0f);
    renderContext.renderPoints = false;
    renderContext.renderNormals = false;
    renderContext.renderOutsideSets = false;
    
    InitializeOpenGL(renderContext);
    
    glClearColor(41.0f / 255.0f, 54.0f / 255.0f, 69.0f / 255.0f, 1.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    inputState.mouseYaw = -90.0f;
    inputState.mousePitch = -45.0f;
    
    CreateLight(renderContext, glm::vec3(0.0f, 50.0f, 30.0f), glm::vec3(1, 1, 1), 2000.0f);
    
    int numberOfPoints = 30000;
    
    vertex* naiveVertices = nullptr;
    vertex* quickHullVertices = nullptr;
    vertex* finalQHVertices = nullptr;
    
    auto vertices = GenerateNewPointSet(renderContext, &naiveVertices, &quickHullVertices, &finalQHVertices, numberOfPoints, 0.0, 100.0);
    
    auto disableMouse = false;
    
    mesh* mq = nullptr; 
    mesh* mFinal = nullptr;
    
    //auto& mn = NaiveConvexHull(renderContext, naiveVertices, numberOfPoints);
    auto& mn = InitEmptyMesh(renderContext);
    
    mesh* currentMesh = mq;
    vertex* currentVertices = vertices;
    
    qh_context qhContext = {};
    InitializeQHContext(qhContext, vertices, numberOfPoints);
    
    qh_context stepQHTimerContext = {};
    InitializeQHContext(stepQHTimerContext, vertices, numberOfPoints);
    double stepQHTimer = 0.0;
    double stepQHTimerInit = 0.01;
    
    
    bool fullTimerStarted = false;
    auto fullQHTimer = 0.0;
    auto fullQHTimerInit = 0.3;
    mesh* fullQHMesh = nullptr;
    vertex* fullQHVertices = nullptr;
    
    bool stepTimerStarted = false;
    
    // Check if the ESC key was pressed or the window was closed
    while(!KeyDown(Key_Escape) &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime);
        
        if(KeyDown(Key_Y))
        {
            if(vertices)
            {
                free(vertices);
            }
            
            if(naiveVertices)
            {
                free(naiveVertices);
            }
            
            if(quickHullVertices)
            {
                free(quickHullVertices);
            }
            
            if(finalQHVertices)
            {
                free(finalQHVertices);
            }
            
            vertices = GenerateNewPointSet(renderContext, &naiveVertices, &quickHullVertices, &finalQHVertices, numberOfPoints, 0.0, 100.0);
            InitializeQHContext(qhContext, vertices, numberOfPoints);
            InitializeQHContext(stepQHTimerContext, vertices, numberOfPoints);
            currentVertices = vertices;
            if(mFinal)
            {
                free(mFinal->faces);
            }
            
            mFinal = nullptr;
            currentMesh = nullptr;
        }
        
        
        if(KeyDown(Key_H))
        {
            if(!mFinal)
            {
                auto before = glfwGetTime();
                mFinal = &QuickHull(renderContext, finalQHVertices, numberOfPoints);
                auto after = glfwGetTime();
                auto total = after - before;
                Log_A("QuickHull took: %fs\n", total);
            }
            if(mFinal)
            {
                currentMesh = mFinal;
                currentVertices = finalQHVertices;
            }
        }
        
        if(KeyDown(Key_J))
        {
            QuickHullStep(renderContext, qhContext);
            currentMesh = &qhContext.m;
            currentVertices = qhContext.vertices;
        }
        
        if(fullTimerStarted)
        {
            if(fullQHTimer <= 0.0)
            {
                if(fullQHVertices)
                {
                    free(fullQHVertices);
                }
                
                fullQHVertices = GeneratePointsInSphere(renderContext, numberOfPoints, 0.0, 100.0);
                fullQHMesh = &QuickHull(renderContext, fullQHVertices, numberOfPoints);
                currentMesh = fullQHMesh;
                currentVertices = fullQHVertices;
                fullQHTimer = fullQHTimerInit;
            }
            else
            {
                fullQHTimer -= deltaTime;
            }
        }
        
        if(KeyDown(Key_R))
        {
            fullTimerStarted= !fullTimerStarted;
            if(fullTimerStarted)
            {
                fullQHTimer = fullQHTimerInit;
            }
            currentMesh = fullQHMesh;
            currentVertices = fullQHVertices;
        }
        
        if(stepTimerStarted)
        {
            if(stepQHTimer <= 0.0)
            {
                QuickHullStep(renderContext, stepQHTimerContext);
            }
            else
            {
                stepQHTimer -= deltaTime;
            }
        }
        
        if(KeyDown(Key_F))
        {
            stepTimerStarted = !stepTimerStarted;
            if(stepTimerStarted)
            {
                stepQHTimer = stepQHTimerInit;
            }
            currentMesh = &stepQHTimerContext.m;
            currentVertices = stepQHTimerContext.vertices;
        }
        
        if(KeyDown(Key_P))
        {
            renderContext.renderPoints = !renderContext.renderPoints;
        }
        
        if(KeyDown(Key_N))
        {
            renderContext.renderNormals = !renderContext.renderNormals;
        }
        
        if(KeyDown(Key_O))
        {
            renderContext.renderOutsideSets = !renderContext.renderOutsideSets;
        }
        
        if(renderContext.renderPoints)
        {
            RenderPointCloud(renderContext, currentVertices, numberOfPoints);
        }
        
        if(KeyDown(Key_Q))
        {
            currentMesh = &qhContext.m; //mq;
            currentVertices = vertices;
        }
        
        if(KeyDown(Key_B))
        {
            currentMesh = &mn;
        }
        
        if(currentMesh)
        {
            RenderMesh(renderContext, *currentMesh, currentVertices);
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

