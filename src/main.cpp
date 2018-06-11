#include <ctime>
#include <chrono>
#include <random>
#include <vector>

#include <cstdio>
#include <cstdlib>
#if defined(__linux)
#include <unistd.h>
#else
#include "Shlwapi.h"
#endif

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <algorithm>


// BEGIN IGNORE WARNINGS IN LIBS ON Windows
#ifdef _WIN32
#pragma warning(push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// END IGNORE WARNINGS  IN LIBS ON Windows
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "list.h"
#include "timing.h"
#include "util.h"
#include "keys.h"

const static float globalScale = 0.1f;

static InputState inputState;

#include "keys.cpp"

#include "rendering.h"
#include "rendering.cpp"

#include "quickhull.h"
#include "incremental.h"
#include "divideconquer.h"

#include "point_generator.h"
#include "hull.h"

void reinitPoints(Vertex **vertices, ConfigData &configData, Hull &h, RenderContext &renderContext)
{
    if(*vertices)
    {
        free(*vertices);
    }
    
    for(auto &t : configData.qhTestSets)
    {
        free(t.testSet);
    }
    
    for(auto &t : configData.incTestSets)
    {
        free(t.testSet);
    }
    
    for(auto &t : configData.dacTestSets)
    {
        free(t.testSet);
    }
    
    clear(configData.qhTestSets);
    clear(configData.incTestSets);
    clear(configData.dacTestSets);
    
    loadConfig("../.config", configData, renderContext);
    
    if(h.numberOfPoints != configData.numberOfPoints || h.pointGenerator.type != configData.genType)
    {
        initPointGenerator(h.pointGenerator, configData.genType, configData.numberOfPoints, 0.0, 200.0);
    }
    
    *vertices = generate(h.pointGenerator, renderContext.originOffset);
    
    configData.meshVertices = nullptr;
    configData.loadedMesh.faces.clear();
}

void reinitHull(Vertex *vertices, Hull &h, Vertex **currentVertices, Mesh **currentMesh, Mesh **fullHull, Mesh **timedHull, Mesh **stepHull)
{
    reinitializeHull(h, vertices, h.pointGenerator.numberOfPoints);
    
    *currentVertices = vertices;
    
    if(*fullHull)
    {
        for(auto &f : (*fullHull)->faces)
        {
            clear(f.vertices);
        }
    }
    
    if(*stepHull)
    {
        for(auto &f : (*stepHull)->faces)
        {
            clear(f.vertices);
        }
    }
    
    if(*timedHull)
    {
        for(auto &f : (*timedHull)->faces)
        {
            clear(f.vertices);
        }
    }
    
    *currentMesh = nullptr;
    *fullHull = nullptr;
    *timedHull = nullptr;
    *stepHull = nullptr;
}

void renderGenAndHullType(RenderContext &renderContext, Hull &h)
{
    auto orientation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    auto scale = glm::vec3(100.0f, 100.0f, 0.0f);
    auto position = glm::vec3(0.0f, 0.0f, 0.0f);
    auto isUi = true;
    switch(h.pointGenerator.type)
    {
        case GeneratorType::InSphere:
        {
            RenderQuad(renderContext, position, orientation, scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), isUi);
        }
        break;
        case GeneratorType::OnSphere:
        {
            RenderQuad(renderContext, position, orientation, scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), isUi);
        }
        break;
        case GeneratorType::InCube:
        {
            RenderQuad(renderContext, position, orientation, scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), isUi);
        }
        break;
        case GeneratorType::NormalizedSphere:
        {
            RenderQuad(renderContext, position, orientation, scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), isUi);
        }
        break;
        case GeneratorType::ManyInternal:
        {
            RenderQuad(renderContext, position, orientation, scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), isUi);
        }
        break;
        case GeneratorType::Clusters:
        {
            RenderQuad(renderContext, position, orientation, scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), isUi);
        }
        break;
    }
}

int main()
{
    auto seed = time(NULL);
    
    srand((unsigned int)seed);
    
    printf("Seed: %zd\n", seed);
    
    RenderContext renderContext = {};
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
    
    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    inputState.mouseYaw = -90.0f;
    inputState.mousePitch = -45.0f;
    
    auto disableMouse = false;
    
    CreateDirectionalLight(renderContext, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.05f), glm::vec3(0.4f), glm::vec3(0.5f));
    CreateSpotLight(renderContext, glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(0.0f, 80.0f, 80.0f), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f)), 1.0f, 0.0014f, 0.000007f);
    
    HullType hullType = HullType::QH;
    
    ConfigData configData = {};
    loadConfig("../.config", configData, renderContext);
    
    Hull h;
    h.vertices = nullptr;
    h.numberOfPoints = 0;
    h.qhContext = {};
    h.stepQhContext = {};
    h.timedStepQhContext = {};
    h.qhTimer = {};
    h.incContext = {};
    h.stepIncContext = {};
    h.timedStepIncContext = {};
    h.dacContext = {};
    h.stepDacContext = {};
    h.timedStepDacContext = {};
    h.incTimer = {};
    h.currentHullType = HullType::QH;
    auto previousHullType = h.currentHullType;
    
    std::random_device rd{};
    std::mt19937_64 gen{rd()};
    gen.seed((unsigned int)seed);
    h.pointGenerator.gen = gen;
    
    int numberOfPoints;
    Vertex *vertices;
    
    if(!configData.meshVertices)
    {
        numberOfPoints = configData.numberOfPoints;
        
        vertices = generate(h.pointGenerator, renderContext.originOffset);
    }
    else
    {
        numberOfPoints = configData.verticesInMesh;
        vertices = configData.meshVertices;
    }
    
    initPointGenerator(h.pointGenerator, configData.genType, numberOfPoints, 0.0, 200.0);
    
    InitializeHull(h, vertices, h.pointGenerator.numberOfPoints, hullType);
    
    Mesh* currentMesh = nullptr;
    Vertex* currentVertices = vertices;
    
    auto fps = 0.0;
    auto currentFrameCount = 0;
    
    auto totalDelta = 0.0;
    
    Mesh* fullHull = nullptr;
    Mesh* timedHull = nullptr;
    Mesh* stepHull = nullptr;
    
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
        
        if(KeyDown(Key_T))
        {
            for(size_t i = 0; i < configData.qhTestSets.size; i++)
            {
                RunFullHullTestQh(configData.qhTestSets[i], renderContext.originOffset);
            }
            
            for(size_t i = 0; i < configData.incTestSets.size; i++)
            {
                RunFullHullTestInc(configData.incTestSets[i], renderContext.originOffset);
            }
            
            for(size_t i = 0; i < configData.dacTestSets.size; i++)
            {
                RunFullHullTestDac(configData.dacTestSets[i], renderContext.originOffset);
            }
            
        }
        
        if(KeyDown(Key_Y))
        {
            reinitPoints(&vertices, configData, h, renderContext);
            reinitHull(vertices, h, &currentVertices, &currentMesh, &fullHull, &timedHull, &stepHull);
        }
        
        if(KeyDown(Key_C))
        {
            reinitHull(vertices, h, &currentVertices, &currentMesh, &fullHull, &timedHull, &stepHull);
        }
        
        if(KeyDown(Key_H))
        {
            if(!fullHull || previousHullType != hullType)
            {
                fullHull = &FullHull(renderContext, h);
                printf("Num faces in mesh: %zd\n", fullHull->faces.size());
            }
            currentMesh = fullHull;
        }
        
        if(configData.meshVertices)
        {
            RenderMesh(renderContext, configData.loadedMesh);
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
            previousHullType = hullType;
            hullType = HullType::QH;
        }
        
        if(KeyDown(Key_I))
        {
            previousHullType = hullType;
            hullType = HullType::Inc;
        }
        
        if(KeyDown(Key_L))
        {
            previousHullType = hullType;
            hullType = HullType::Dac;
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
        
        renderGenAndHullType(renderContext, h);
        
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

