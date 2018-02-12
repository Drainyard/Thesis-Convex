    static void MousePositionCallback(GLFWwindow * window, double xPos, double yPos)
    {
        inputState.xDelta = inputState.xPos - xPos;
        inputState.yDelta = inputState.yPos - yPos;
        inputState.xPos = xPos;
        inputState.yPos = yPos;
    }
    
    static void MouseScrollCallback(GLFWwindow* window, double xScroll, double yScroll)
    {
        inputState.xScroll = xScroll;
        inputState.yScroll = yScroll;
    }
    
