#ifndef KEYS_H
#define KEYS_H

#define Key_Left GLFW_KEY_LEFT
#define Key_Right GLFW_KEY_RIGHT
#define Key_Up GLFW_KEY_UP
#define Key_Down GLFW_KEY_DOWN
#define Key_Escape GLFW_KEY_ESCAPE
#define Key_F1 GLFW_KEY_F1
#define Key_F2 GLFW_KEY_F2
#define Key_F3 GLFW_KEY_F3
#define Key_F4 GLFW_KEY_F4
#define Key_F5 GLFW_KEY_F5
#define Key_F6 GLFW_KEY_F6
#define Key_F7 GLFW_KEY_F7
#define Key_F8 GLFW_KEY_F8
#define Key_F9 GLFW_KEY_F9
#define Key_F10 GLFW_KEY_F10
#define Key_F11 GLFW_KEY_F11
#define Key_F12 GLFW_KEY_F12
#define Key_F13 GLFW_KEY_F13
#define Key_F14 GLFW_KEY_F14
#define Key_F15 GLFW_KEY_F15
#define Key_F16 GLFW_KEY_F16
#define Key_F17 GLFW_KEY_F17
#define Key_F18 GLFW_KEY_F18
#define Key_F19 GLFW_KEY_F19
#define Key_F20 GLFW_KEY_F20
#define Key_F21 GLFW_KEY_F21
#define Key_F22 GLFW_KEY_F22
#define Key_F23 GLFW_KEY_F23
#define Key_F24 GLFW_KEY_F24
#define Key_F25 GLFW_KEY_F25
#define Key_A GLFW_KEY_A
#define Key_B GLFW_KEY_B
#define Key_C GLFW_KEY_C
#define Key_D GLFW_KEY_D
#define Key_E GLFW_KEY_E
#define Key_F GLFW_KEY_F
#define Key_G GLFW_KEY_G
#define Key_H GLFW_KEY_H
#define Key_I GLFW_KEY_I
#define Key_J GLFW_KEY_J
#define Key_K GLFW_KEY_K
#define Key_L GLFW_KEY_L
#define Key_M GLFW_KEY_M
#define Key_N GLFW_KEY_N
#define Key_O GLFW_KEY_O
#define Key_P GLFW_KEY_P
#define Key_Q GLFW_KEY_Q
#define Key_R GLFW_KEY_R
#define Key_S GLFW_KEY_S
#define Key_T GLFW_KEY_T
#define Key_U GLFW_KEY_U
#define Key_V GLFW_KEY_V
#define Key_W GLFW_KEY_W
#define Key_X GLFW_KEY_X
#define Key_Y GLFW_KEY_Y
#define Key_Z GLFW_KEY_Z
#define Key_Space GLFW_KEY_SPACE
#define Key_Tab GLFW_KEY_TAB
#define Key_Enter GLFW_KEY_ENTER
#define Key_Backspace GLFW_KEY_BACKSPACE
#define Key_LeftShift GLFW_KEY_LEFT_SHIFT
#define Key_LeftCtrl GLFW_KEY_LEFT_CTRL
#define Key_RightCtrl GLFW_KEY_RIGHT_CTRL
#define Key_RightShift GLFW_KEY_RIGHT_SHIFT
#define Key_Delete GLFW_KEY_DELETE
#define Key_0 GLFW_KEY_0
#define Key_1 GLFW_KEY_1
#define Key_2 GLFW_KEY_2
#define Key_3 GLFW_KEY_3
#define Key_4 GLFW_KEY_4
#define Key_5 GLFW_KEY_5
#define Key_6 GLFW_KEY_6
#define Key_7 GLFW_KEY_7
#define Key_8 GLFW_KEY_8
#define Key_9 GLFW_KEY_9
#define Key_Add GLFW_KEY_ADD
#define Key_Subtract GLFW_KEY_SUBTRACT

#define Key_Unknown GLFW_KEY_UNKNOWN
#define Key_Apostrophe GLFW_KEY_APOSTROPHE
#define Key_Comma GLFW_KEY_COMMA
#define Key_Minus GLFW_KEY_MINUS
#define Key_Period GLFW_KEY_PERIOD
#define Key_Slash GLFW_KEY_SLASH
#define Key_SemiColon GLFW_KEY_SEMI_COLON
#define Key_Equal GLFW_KEY_EQUAL
#define Key_LeftBracket GLFW_KEY_LEFT_BRACKET
#define Key_RightBracket GLFW_KEY_RIGHT_BRACKET
#define Key_ 89
#define Key_GraveAccent GLFW_KEY_GRAVE_ACCENT
#define Key_World1 GLFW_KEY_WORLD1
#define Key_World2 GLFW_KEY_WORLD2
#define Key_Insert GLFW_KEY_INSERT
#define Key_PageUp GLFW_KEY_PAGE_UP
#define Key_PageDown GLFW_KEY_PAGE_DOWN
#define Key_Home GLFW_KEY_HOME
#define Key_Backslash GLFW_KEY_BACKSLASH
#define Key_End GLFW_KEY_END
#define Key_CapsLock GLFW_KEY_CAPS_LOCK
#define Key_ScrollLock GLFW_KEY_SCROLL_LOCK
#define Key_NumLock GLFW_KEY_NUM_LOCK
#define Key_PrintScreen GLFW_KEY_PRINT_SCREEN
#define Key_Pause GLFW_KEY_PAUSE
#define Key_LeftAlt GLFW_KEY_LEFT_ALT
#define Key_RightAlt GLFW_KEY_RIGHT_ALT
#define Key_LeftSuper GLFW_KEY_LEFT_SUPER
#define Key_RightSuper GLFW_KEY_RIGHT_SUPER
#define Key_Menu GLFW_KEY_MENU
#define Key_KP0 GLFW_KEY_KP0
#define Key_KP1 GLFW_KEY_KP1
#define Key_KP2 GLFW_KEY_KP2
#define Key_KP3 GLFW_KEY_KP3
#define Key_KP4 GLFW_KEY_KP4
#define Key_KP5 GLFW_KEY_KP5
#define Key_KP6 GLFW_KEY_KP6
#define Key_KP7 GLFW_KEY_KP7
#define Key_KP8 GLFW_KEY_KP8
#define Key_KP9 GLFW_KEY_KP9
#define Key_KPDecimal GLFW_KEY_KPDECIMAL
#define Key_KPDivide GLFW_KEY_KPDIVIDE
#define Key_KPMultiply GLFW_KEY_KPMULTIPLY
#define Key_KPEnter GLFW_KEY_KPENTER
#define Key_KPEqual GLFW_KEY_KPEQUAL
#define Key_Last Key_KPEqual


#define MouseLeft GLFW_MOUSE_BUTTON_LEFT
#define MouseRight GLFW_MOUSE_BUTTON_RIGHT
#define MouseMiddle GLFW_MOUSE_BUTTON_MIDDLE

enum KeyState
{
    Key_NotPressed,
    Key_Pressed,
    Key_JustPressed,
    Key_Invalid
};

struct InputState
{
    float xPos;
    float yPos;
    
    float xDelta;
    float yDelta;
    
    float xScroll;
    float yScroll;
    
    float sensitivity = 0.05f;
    
    float mouseYaw;
    float mousePitch;
    
    bool firstMouse;
    
    KeyState keys[GLFW_KEY_LAST];
    KeyState mouseButtons[GLFW_MOUSE_BUTTON_LAST]; // For now. Add more later?
    bool keysDown[GLFW_KEY_LAST];
    bool mouseButtonsDown[GLFW_MOUSE_BUTTON_LAST];
};

#endif
