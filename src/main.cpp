#include <ctime>
#include <random>
#include <vector>
#include <map>
#include <stack>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <algorithm>

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

#include "timing.h"
#include "util.h"
#include "keys.h"

const static float globalScale = 0.1f;

static input_state inputState;

#include "keys.cpp"

#include "rendering.h"
#include "rendering.cpp"

#include "quickhull.h"
#include "incremental.h"

#include "point_generator.h"
#include "hull.h"


#define Min(A,B) ((A < B) ? (A) : (B))
#define Max(A,B) ((A > B) ? (A) : (B))
#define Abs(x) ((x) < 0 ? -(x) : (x))


int main()
{
    // Degenerate: 1520515408
    //auto seed = 1521294278;
    auto seed = time(NULL);
    srand((unsigned int)seed);
    printf("Seed: %zd\n", seed);
    render_context renderContext = {};
    renderContext.FoV = 45.0f;
    renderContext.position = glm::vec3(0.0f, 50.5f, 70.0f);
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
    //int numberOfPoints = 27948;
    int numberOfPoints = 10000;
    
    hull h;
    std::random_device rd{};
    std::mt19937 gen{rd()};
    h.pointGenerator.gen = gen;
    
    InitPointGenerator(h.pointGenerator, GeneratorType::ManyInternal, numberOfPoints);
    
    auto vertices = Generate(h.pointGenerator, renderContext, 0.0f, 200.0f);
    //auto vertices = LoadObj("../assets/obj/big boi arnold 17500.OBJ");
    //auto vertices = LoadObj("../assets/obj/man in vest 650k.OBJ");
    //auto vertices = LoadObj("../assets/obj/CarpetBit.obj");
    
    InitializeHull(h, vertices, h.pointGenerator.numberOfPoints, hullType);
    
    mesh* currentMesh = nullptr;
    vertex* currentVertices = vertices;
    
    auto fps = 0.0;
    auto currentFrameCount = 0;
    
    auto totalDelta = 0.0;
    
    mesh* fullHull = nullptr;
    mesh* timedHull = nullptr;
    mesh* stepHull = nullptr;
    
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
        }
        
        totalDelta += deltaTime;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime);
        
        auto updatedMesh = UpdateHull(renderContext, h, hullType, deltaTime);
        if(updatedMesh)
        {
            currentMesh = updatedMesh;
        }
        
        if(KeyDown(Key_Y))
        {
            if(vertices)
            {
                free(vertices);
            }
            
            vertices = Generate(h.pointGenerator, renderContext, 0.0f, 200.0f);
            ReinitializeHull(h, vertices, h.pointGenerator.numberOfPoints);
            
            currentVertices = vertices;
            currentMesh = nullptr;
            fullHull = nullptr;
            timedHull = nullptr;
            stepHull = nullptr;
        }
        
        if(KeyDown(Key_H))
        {
            if(!fullHull)
                fullHull = &FullHull(renderContext, h);
            currentMesh = fullHull;
        }
        
        if(KeyDown(Key_J))
        {
            stepHull = &StepHull(renderContext, h);
            currentMesh = stepHull;
        }
        
        if(KeyDown(Key_F))
        {
            timedHull = &TimedStepHull(renderContext, h);
            currentMesh = timedHull;
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
            RenderPointCloud(renderContext, currentVertices, h.pointGenerator.numberOfPoints);
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
            RenderMesh(renderContext, *currentMesh);
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

