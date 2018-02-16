#include <ctime>
#include <random>
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

#include "keys.h"

static input_state inputState;

#include "keys.cpp"

#include "rendering.h"
#include "rendering.cpp"

#define Min(A,B) ((A < B) ? (A) : (B))
#define Max(A,B) ((A > B) ? (A) : (B))
#define Abs(x) ((x) < 0 ? -(x) : (x))

static float RandomFloat(float start, float end)
{
    return (rand() / (float)RAND_MAX * end) + start;
}

static int RandomInt(int start, int end)
{
    return (rand() % end) + start;
}

static glm::vec4 RandomColor()
{
    return glm::vec4((float)RandomInt(0, 255) / 255, (float)RandomInt(0, 255) / 255, (float)RandomInt(0, 255) / 255, (float)RandomInt(0, 255) / 255);
}

struct point
{
    glm::vec3 position;
    glm::vec4 color;
};

static point* GeneratePoints(render_context& renderContext, int numberOfPoints)
{
    auto res = (point*)malloc(sizeof(point) * numberOfPoints);
    for(int i = 0; i < numberOfPoints; i++)
    {
        float x = RandomFloat(0, 100);
        float y = RandomFloat(0, 100);
        float z = RandomFloat(0, 100);
        
        res[i].position = glm::vec3(x, y, z) - renderContext.originOffset;
        res[i].color = RandomColor();
    }
    return res;
}

int main()
{
    srand(time(NULL));
    render_context renderContext = {};
    renderContext.FoV = 45.0f;
    renderContext.position = glm::vec3(0.0f, 30.5f, 30.0f);
    renderContext.direction = glm::vec3(0.0f, -0.75f, -1.0f);
    renderContext.up = glm::vec3(0.0f, 1.0f, 0.0f);
    renderContext.near = 0.1f;
    renderContext.far =  1000.0f;
    renderContext.originOffset = glm::vec3(5.0f, 0.0f, 5.0f);
    
    InitializeOpenGL(renderContext);
    
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    
    double lastFrame = glfwGetTime();
    double currentFrame = 0.0;
    double deltaTime;
    inputState.mouseYaw = -90.0f;
    inputState.mousePitch = -45.0f;
    
    CreateLight(renderContext, glm::vec3(0.0f, 7.5f, 10.0f), glm::vec3(1, 1, 1), 25.0f);
    
    int numberOfPoints = 1000;
    
    auto globalScale = 0.2f;
    
    /////////////// DEBUG
    auto points = GeneratePoints(renderContext, numberOfPoints);
    
    auto c = (points[0].position + points[1].position + points[2].position +points[3].position + points[4].position + points[5].position)/6.0f;
    auto p1Obj = points[0].position - c;
    auto p2Obj = points[1].position - c;
    auto p3Obj = points[2].position - c;
    auto p4Obj = points[3].position - c;
    auto p5Obj = points[4].position - c;
    auto p6Obj = points[5].position - c;
    
    //auto& m1 = LoadModel(renderContext, vbo, 0, 0, 0, glm::vec3(0.5f, 0.5f, 0.0f));
    auto& m1 = InitEmptyModel(renderContext);
    m1.position = c;
    m1.scale = glm::vec3(globalScale);
    m1.numFaces = 0;
    m1.facesSize = 0;
    AddFace(m1, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), p3Obj);
    AddFace(m1, p4Obj, p5Obj, p6Obj);
    /////////////// END DEBUG
    
    // Check if the ESC key was pressed or the window was closed
    while(glfwGetKey(renderContext.window, Key_Escape ) != GLFW_PRESS &&
          glfwWindowShouldClose(renderContext.window) == 0 )
    {
        currentFrame = glfwGetTime();
        deltaTime = Min(currentFrame - lastFrame, 0.1);
        lastFrame = currentFrame;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ComputeMatrices(renderContext, deltaTime);
        
        for(int p = 0; p < numberOfPoints; p++)
        {
            RenderQuad(renderContext, points[p].position, glm::quat(0.0f, 0.0f, 0.0f, 0.0f), glm::vec3(globalScale), points[p].color);
        }
        
        Render(renderContext);
        
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

