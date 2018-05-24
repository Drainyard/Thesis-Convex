#ifndef RENDERING_H
#define RENDERING_H

struct texture
{
    int textureID;
    int width;
    int height;
    unsigned char* data;
};

struct shader
{
    GLuint programID;
};

enum LightType
{
    LT_DIRECTIONAL,
    LT_SPOT
};

struct light
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

enum Material_Type
{
    MT_color,
    MT_texture
};

struct render_material
{
    Material_Type type;
    union
    {
        struct
        {
            glm::vec3 color;
        } diffuse;
        struct
        {
            texture tex;
        } texture;
    };
    
    glm::vec3 specularColor;
    float alpha;
    
    shader materialShader;
};

struct vertex_info
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
};

struct vertex
{
    union
    {
        vertex_info info;
        struct
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec4 color;
        };
    };
    int vertexIndex;
};

struct face
{
    List<vertex> vertices;
    
    glm::vec3 faceNormal;
    glm::vec4 faceColor;
    
    glm::vec3 centerPoint;
};

struct mesh
{
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;
    glm::mat4 transform;
    
    int meshIndex = -1;
    render_material material;
    
    bool dirty;
    GLfloat* currentVBO;
    
    std::vector<face> faces;
    
    GLuint VAO;
    GLuint VBO;
    
    int vertexCount;
};

struct debug_context
{
    int currentFaceIndex;
    int currentDistantPoint;
    std::vector<edge> horizon;
};

struct render_context
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    
    debug_context debugContext;
    
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
    
    shader textureShader;
    shader colorShader;
    shader basicShader;
    shader particleShader;
    shader lineShader;
    
    mesh meshes[64];
    int meshCount;
    
    light lights[64];
    int lightCount;
    
    bool renderPoints;
    bool renderNormals;
    bool renderOutsideSets;
    
    edge nonConvexEdge;
    
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
