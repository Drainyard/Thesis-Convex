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
    int programID;
};

enum Buffer_Type
{
    BT_vertexBuffer,
    BT_uvBuffer,
    BT_colorBuffer,
    BT_normalBuffer
};

struct gl_buffer
{
    Buffer_Type type;
    GLfloat* data;
    size_t size;
    int count;
    union
    {
        struct
        {
        } vbo;
        struct
        {
            texture tex;
        } uv;
        struct
        {
        } color;
        struct
        {
        } normal;
    };
};

struct light
{
    glm::vec3 position;
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
            glm::vec3 diffuseColor;
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
    //int vertices[3];
    vertex vertices[3];
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
