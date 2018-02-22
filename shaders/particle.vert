#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 instanced_pos;
layout(location = 2) in vec4 instanced_color;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec4 c;

void main()
{
	gl_Position = P * V * M * vec4(position + instanced_pos, 1);
	c = instanced_color;
}



