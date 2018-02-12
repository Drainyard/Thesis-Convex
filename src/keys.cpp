    static void MousePositionCallback(GLFWwindow * window, double xPos, double yPos)
    {
        
        if(inputState.firstMouse) // this bool variable is initially set to true
        {
            inputState.xPos = xPos;
            inputState.yPos = yPos;
            inputState.firstMouse = false;
        }
        
        inputState.xDelta = (xPos - inputState.xPos) * inputState.sensitivity;
        inputState.yDelta = (inputState.yPos - yPos) * inputState.sensitivity;
        inputState.xPos = xPos;
        inputState.yPos = yPos;
        
        if(glfwGetMouseButton(window, Key_MouseLeft) == GLFW_PRESS)
        {
            inputState.mouseYaw += inputState.xDelta;
            inputState.mousePitch += inputState.yDelta;
        }
        
        if(inputState.mousePitch > 89.0f)
            inputState.mousePitch=  89.0f;
        if(inputState.mousePitch < -89.0f)
            inputState.mousePitch= -89.0f;
    }
    
    static void MouseScrollCallback(GLFWwindow* window, double xScroll, double yScroll)
    {
        inputState.xScroll = xScroll;
        inputState.yScroll = yScroll;
    }
    
