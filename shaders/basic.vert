#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

uniform int isUI;

void main()
{
    if(isUI == 1)
    {
        gl_Position = V * M * vec4(position.xy, 0.0f, 1.0f);
    }
    else
    {
        gl_Position = P * V * M * vec4(position, 1);
    }
    
}

