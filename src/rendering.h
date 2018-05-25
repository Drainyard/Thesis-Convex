#ifndef RENDERING_H
#define RENDERING_H

struct Texture
{
    int textureID;
    int width;
    int height;
    unsigned char* data;
};

struct Shader
{
    GLuint programID;
};

enum LightType
{
    LT_DIRECTIONAL,
    LT_SPOT
};

struct Light
{
    LightType lightType;
    union
    {
        struct
        {
            float cutOff;
            float outerCutOff;
            float constant;
            float linear;
            float quadratic;
            glm::vec3 position;
        } spot;
    };
    
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 direction;
    glm::vec3 color;
    float power;
};

enum MaterialType
{
    MT_color,
    MT_texture
};

struct RenderMaterial
{
    MaterialType type;
    union
    {
        struct
        {
            glm::vec3 color;
        } diffuse;
        struct
        {
            Texture tex;
        } texture;
    };
    
    glm::vec3 specularColor;
    float alpha;
    
    Shader materialShader;
};

struct VertexInfo
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
};

struct Vertex
{
    union
    {
        VertexInfo info;
        struct
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec4 color;
        };
    };
    int vertexIndex;
};

struct Face
{
    List<Vertex> vertices;
    
    glm::vec3 faceNormal;
    glm::vec4 faceColor;
    
    glm::vec3 centerPoint;
};

struct Mesh
{
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;
    glm::mat4 transform;
    
    int meshIndex = -1;
    RenderMaterial material;
    
    bool dirty;
    GLfloat* currentVBO;
    
    std::vector<Face> faces;
    
    GLuint VAO;
    GLuint VBO;
    
    size_t vertexCount;
};

struct DebugContext
{
    int currentFaceIndex;
    int currentDistantPoint;
    std::vector<Edge> horizon;
};

struct RenderContext
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    
    DebugContext debugContext;
    
    float FoV;
    
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 right;
    
    float nearPlane;
    float farPlane;
    
    glm::vec3 originOffset;
    
    int screenWidth;
    int screenHeight;
    
    GLFWwindow* window;
    
    Shader textureShader;
    Shader colorShader;
    Shader basicShader;
    Shader particleShader;
    Shader lineShader;
    
    Mesh meshes[64];
    int meshCount;
    
    Light lights[64];
    int lightCount;
    
    bool renderPoints;
    bool renderNormals;
    bool renderOutsideSets;
    
    Edge nonConvexEdge;
    
    GLuint primitiveVAO;
    GLuint primitiveVBO;
    
    GLuint lineVAO;
    GLuint lineVBO;
    GLuint lineEBO;
    
#define LINE_INDICES 6
    
    GLuint lineIndices[LINE_INDICES] = 
    {
        0, 1, 2,
        1, 2, 3
    };
    
    GLuint quadVAO;
    GLuint quadVBO;
    GLuint quadIndexBuffer;
    GLuint quadIndices[6] = {0, 1, 2, 2, 1, 3};
    GLfloat quadVertices[8] = 
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
    
    GLuint pointCloudVAO;
    GLuint particlesVBO;
    GLuint particlesColorVBO;
    GLuint billboardVBO;
};

#endif
