#include <ctime>
#include <random>
#include <vector>
#include <map>
#include <stack>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <algorithm>
#include "timing.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include "GLFW/glfw3.h"

// BEGIN IGNORE WARNINGS IN LIBS ON Windows
#ifdef _WIN32
#pragma warning(push, 0)
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// END IGNORE WARNINGS  IN LIBS ON Windows
#ifdef _WIN32
#pragma warning(pop)
#endif


#include "util.h"
#include "keys.h"

const static float globalScale = 0.2f;

static input_state inputState;

#include "keys.cpp"

#include "rendering.h"
#include "rendering.cpp"

#include "quickhull.h"
#include "incremental.h"

#include "hull.h"

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
    //auto vertices = LoadObj("../assets/obj/man in vest 40k.OBJ");
    *naive = CopyVertices(vertices, numVertices);
    *quickhull = CopyVertices(vertices, numVertices);
    *user = CopyVertices(vertices, numVertices);
    return vertices;
}

int main()
{
    // Degenerate: 1520515408
    //auto seed = 1521294278;
    auto seed = time(NULL);
    srand((unsigned int)seed);
    printf("Seed: %zd\n", seed);
    render_context renderContext = {};
    renderContext.FoV = 45.0f;
    renderContext.position = glm::vec3(0.0f, 170.5f, 230.0f);
    renderContext.direction = glm::vec3(0.0f, -0.75f, -1.0f);
    renderContext.up = glm::vec3(0.0f, 1.0f, 0.0f);
    renderContext.nearPlane = 0.1f;
    renderContext.farPlane =  10000.0f;
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
    
    auto disableMouse = false;
    
    CreateLight(renderContext, glm::vec3(0.0f, 80.0f, 80.0f), glm::vec3(1, 1, 1), 2000.0f);
    CreateLight(renderContext, glm::vec3(10.0f, -20.0f, 20.0f), glm::vec3(1, 1, 1), 2000.0f);
    
    HullType hullType = HullType::QH;
    
    //int numberOfPoints = 645932; // Man in vest numbers
    //int numberOfPoints = 17536; // Arnold
    int numberOfPoints = 10000;
    auto vertices = GeneratePointsInSphere(renderContext, numberOfPoints, 0.0f, 200.0f);
    //auto vertices = LoadObj("../assets/obj/big boi arnold 17500.OBJ");
    //auto vertices = LoadObj("../assets/obj/man in vest 650k.OBJ");
    
    hull h = {};
    InitializeHull(h, vertices, numberOfPoints, hullType);
    
    mesh* currentMesh = nullptr;
    vertex* currentVertices = vertices;
    
    auto fps = 0.0;
    auto currentFrameCount = 0;
    
    auto totalDelta = 0.0;
    
    // Check if the ESC key was pressed or the window was closed
    while(!KeyDown(Key_Escape) &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        currentFrameCount++;
        
        if(totalDelta >= 1.0)
        {
            fps = (1.0 * (1.0 - totalDelta)) + (double)currentFrameCount;
            totalDelta = 0.0;
            currentFrameCount = 0;
            //Log_A("FPS: %f\n", fps);
        }
        
        totalDelta += deltaTime;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime);
        
        UpdateHull(renderContext, h, hullType, deltaTime);
        
        if(KeyDown(Key_Y))
        {
            if(vertices)
            {
                free(vertices);
            }
            
            vertices = GeneratePointsInSphere(renderContext, numberOfPoints, 0.0f, 200.0f);
            ReinitializeHull(h, vertices, numberOfPoints);
            
            currentVertices = vertices;
            currentMesh = nullptr;
        }
        
        if(KeyDown(Key_H))
        {
            currentMesh = &FullHull(renderContext, h);
        }
        
        if(KeyDown(Key_J))
        {
            currentMesh = &StepHull(renderContext, h);
        }
        
        if(KeyDown(Key_F))
        {
            currentMesh = &TimedStepHull(h);
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
            hullType = HullType::QH;
        }
        
        if(KeyDown(Key_I))
        {
            hullType = HullType::Inc;
        }
        
        if(KeyDown(Key_R))
        {
            hullType = HullType::RInc;
        }
        
        if(currentMesh)
        {
            RenderMesh(renderContext, *currentMesh, currentVertices, deltaTime);
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

