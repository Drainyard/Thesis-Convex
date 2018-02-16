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


struct vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
};

struct face
{
    vertex vertices[3];
    glm::vec3 faceNormal;
    glm::vec4 faceColor;
};

struct mesh
{
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;
    glm::mat4 transform;
    
    render_material material;
    
    face* faces;
    int numFaces;
    int facesSize;
    
    GLuint VAO;
    GLuint VBO;
    int vertexCount;
    
    GLuint uvBufferHandle;
    int uvCount;
    bool hasUV;
    
    GLuint colorBufferHandle;
    int colorCount;
    bool hasColor;
    
    GLuint normalBufferHandle;
    int normalCount;
    bool hasNormals;
};


struct render_context
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    
    float FoV;
    
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 right;
    
    float near;
    float far;
    
    glm::vec3 originOffset;
    
    int screenWidth;
    int screenHeight;
    
    GLFWwindow* window;
    
    shader textureShader;
    shader colorShader;
    shader basicShader;
    
    mesh meshes[64];
    int meshCount;
    
    light lights[64];
    int lightCount;
    
    GLuint primitiveVAO;
    GLuint primitiveVBO;
    
    GLuint quadVAO;
    GLuint quadVBO;
    GLuint quadIndexBuffer;
    GLuint quadIndices[6] = {0, 1, 2, 0, 2, 3};
    GLfloat quadVertices[8] = 
    {
        -0.5, -0.5,
        0.5, -0.5,
        0.5, 0.5,
        -0.5, 0.5
    };
};

#endif
