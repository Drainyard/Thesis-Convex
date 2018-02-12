static void ComputeMatrices(render_context& renderer, double deltaTime, glm::vec3 target)
{
    
    renderer.projectionMatrix = glm::perspective(glm::radians(renderer.FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    
    auto scrollSpeed = 10.0f;
    
    renderer.position += glm::vec3(target.x - renderer.position.x * deltaTime * inputState.yScroll * scrollSpeed, target.y - renderer.position.y * deltaTime * inputState.yScroll * scrollSpeed, target.z - renderer.position.z * deltaTime * inputState.yScroll * scrollSpeed);
    renderer.viewMatrix = glm::lookAt(renderer.position, target, glm::vec3(0, 1, 0));
}


static void ComputeModelTransformation(render_context& renderer , double deltaTime, model& object)
{
    if(glfwGetMouseButton(renderer.window, Key_MouseLeft) == GLFW_PRESS)
    {
        object.orientation = glm::vec3(object.orientation.x - (float)inputState.yDelta * (float)deltaTime, object.orientation.y - (float)inputState.xDelta * (float)deltaTime, 0);
        glm::quat qY = glm::quat(object.orientation.y - (float)inputState.yDelta * (float)deltaTime, 0, 1, 0);
        glm::quat qX = glm::quat(object.orientation.x - (float)inputState.xDelta * (float)deltaTime, 1, 0 ,1);
        auto rot = qY * qX;
        object.transform = glm::toMat4(rot);
        object.transform = glm::translate(object.transform, glm::vec3(0, 0, 0));
        
    }
}