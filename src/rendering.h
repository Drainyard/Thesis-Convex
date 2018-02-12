
struct render_context
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    
    float horizontalAngle;
    float verticalAngle;
    float FoV;
    float speed;
    float mouseSpeed;
    
    glm::vec3 position;
    
    int screenWidth;
    int screenHeight;
    
    GLFWwindow* window;
};

struct model
{
    glm::vec3 position;
    glm::vec3 orientation;
    glm::mat4 transform;
};

