                                                                                                                                                
                                                                                                                                                    static void MouseScrollCallback(GLFWwindow* window, double xScroll, double yScroll)
                                                                                                                                                {
                                                                                                                                                    (void)window; // Silence unused warning
                                                                                                                                                    inputState.xScroll = xScroll;
                                                                                                                                                    inputState.yScroll = yScroll;
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
                                                                                                                                                {
                                                                                                                                                    (void)window;
                                                                                                                                                    (void)mods;
                                                                                                                                                    if(action == GLFW_PRESS)
                                                                                                                                                    {
                                                                                                                                                        if(inputState.mouseButtons[button] == Key_NotPressed)
                                                                                                                                                        {
                                                                                                                                                            inputState.mouseButtons[button] = Key_JustPressed;
                                                                                                                                                        }
                                                                                                                                                        inputState.mouseButtonsDown[button] = true;
                                                                                                                                                    }
                                                                                                                                                    else if(action == GLFW_RELEASE)
                                                                                                                                                    {
                                                                                                                                                        inputState.mouseButtons[button] = Key_NotPressed;
                                                                                                                                                        inputState.mouseButtonsDown[button] = false;
                                                                                                                                                    }
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                static bool MouseDown(int button)
                                                                                                                                                {
                                                                                                                                                    return inputState.mouseButtons[button] == Key_JustPressed;
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                static bool Mouse(int button)
                                                                                                                                                {
                                                                                                                                                    return inputState.mouseButtonsDown[button];
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
                                                                                                                                                {
                                                                                                                                                    (void)scancode;
                                                                                                                                                    (void)window;
                                                                                                                                                    (void)mods;
                                                                                                                                                    if(action == GLFW_PRESS)
                                                                                                                                                    {
                                                                                                                                                        if(inputState.keys[key] == Key_NotPressed)
                                                                                                                                                        {
                                                                                                                                                            inputState.keys[key] = Key_JustPressed;
                                                                                                                                                        }
                                                                                                                                                        inputState.keysDown[key] = true;
                                                                                                                                                    }
                                                                                                                                                    else if(action == GLFW_RELEASE)
                                                                                                                                                    {
                                                                                                                                                        inputState.keys[key] = Key_NotPressed;
                                                                                                                                                        inputState.keysDown[key] = false;
                                                                                                                                                    }
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                static bool KeyDown(int key)
                                                                                                                                                {
                                                                                                                                                    if(key == MouseLeft)
                                                                                                                                                        return MouseDown(MouseLeft);
                                                                                                                                                    if(key == MouseRight)
                                                                                                                                                        return MouseDown(MouseRight);
                                                                                                                                                    return inputState.keys[key] == Key_JustPressed;
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                static bool Key(int key)
                                                                                                                                                {
                                                                                                                                                    if(key == MouseLeft)
                                                                                                                                                        return Mouse(MouseLeft);
                                                                                                                                                    if(key == MouseRight)
                                                                                                                                                        return Mouse(MouseRight);
                                                                                                                                                    return inputState.keysDown[key];
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                
                                                                                                                                                static void MousePositionCallback(GLFWwindow * window, double xPos, double yPos)
                                                                                                                                                {
                                                                                                                                                    (void)window;
                                                                                                                                                    if(inputState.firstMouse)
                                                                                                                                                    {
                                                                                                                                                        inputState.xPos = xPos;
                                                                                                                                                        inputState.yPos = yPos;
                                                                                                                                                        inputState.firstMouse = false;
                                                                                                                                                    }
                                                                                                                                                    
                                                                                                                                                    inputState.xDelta = (xPos - inputState.xPos) * inputState.sensitivity;
                                                                                                                                                    inputState.yDelta = (inputState.yPos - yPos) * inputState.sensitivity;
                                                                                                                                                    inputState.xPos = xPos;
                                                                                                                                                    inputState.yPos = yPos;
                                                                                                                                                    
                                                                                                                                                    if(Mouse(MouseLeft))
                                                                                                                                                    {
                                                                                                                                                        inputState.mouseYaw += inputState.xDelta;
                                                                                                                                                        inputState.mousePitch += inputState.yDelta;
                                                                                                                                                    }
                                                                                                                                                    
                                                                                                                                                    if(inputState.mousePitch > 89.0f)
                                                                                                                                                        inputState.mousePitch = 89.0f;
                                                                                                                                                    if(inputState.mousePitch < -89.0f)
                                                                                                                                                        inputState.mousePitch = -89.0f;
                                                                                                                                                }
                                                                                                                                                
                                                                                                                                                static void SetInvalidKeys()
                                                                                                                                                {
                                                                                                                                                    for(int keyCode = 0; keyCode < GLFW_KEY_LAST; keyCode++)
                                                                                                                                                    {
                                                                                                                                                        if(inputState.keys[keyCode] == Key_JustPressed)
                                                                                                                                                        {
                                                                                                                                                            inputState.keys[keyCode] = Key_Invalid;
                                                                                                                                                        }
                                                                                                                                                    }
                                                                                                                                                    
                                                                                                                                                    for(int mouseCode = 0; mouseCode < GLFW_MOUSE_BUTTON_LAST; mouseCode++)
                                                                                                                                                    {
                                                                                                                                                        if(inputState.mouseButtons[mouseCode] == Key_JustPressed)
                                                                                                                                                        {
                                                                                                                                                            inputState.mouseButtons[mouseCode] = Key_Invalid;
                                                                                                                                                        }
                                                                                                                                                    }
                                                                                                                                                }
