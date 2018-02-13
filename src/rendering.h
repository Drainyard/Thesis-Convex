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

struct model
{
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;
    glm::mat4 transform;
    
    render_material material;
    
    GLuint VAO;
    GLuint VBO;
    GLuint uvBufferHandle;
    bool hasUV;
    
    GLuint colorBufferHandle;
    bool hasColor;
    
    GLuint normalBufferHandle;
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
    
    int screenWidth;
    int screenHeight;
    
    GLFWwindow* window;
    
    shader textureShader;
    shader colorShader;
    shader basicShader;
    
    model models[64];
    int modelCount;
    
    light lights[64];
    int lightCount;
    
    GLuint primitiveVAO;
    GLuint primitiveVBO;
};

static GLfloat uvs[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f
    
};

float vertices[] = {
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f, 
    0.5f,  0.5f, -0.5f, 
    0.5f,  0.5f, -0.5f, 
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    
    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f, 
    0.5f,  0.5f,  0.5f, 
    0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    
    0.5f,  0.5f,  0.5f, 
    0.5f,  0.5f, -0.5f, 
    0.5f, -0.5f, -0.5f, 
    0.5f, -0.5f, -0.5f,  
    0.5f, -0.5f,  0.5f, 
    0.5f,  0.5f,  0.5f, 
    
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f, 
    0.5f, -0.5f,  0.5f, 
    0.5f, -0.5f,  0.5f, 
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
    
    -0.5f,  0.5f, -0.5f, 
    0.5f,  0.5f, -0.5f,  
    0.5f,  0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f, 
};

float normals[] = {
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f, 
    0.0f,  0.0f, -1.0f, 
    0.0f,  0.0f, -1.0f, 
    0.0f,  0.0f, -1.0f, 
    0.0f,  0.0f, -1.0f, 
    
    0.0f,  0.0f, 1.0f,
    0.0f,  0.0f, 1.0f,
    0.0f,  0.0f, 1.0f,
    0.0f,  0.0f, 1.0f,
    0.0f,  0.0f, 1.0f,
    0.0f,  0.0f, 1.0f,
    
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f
};