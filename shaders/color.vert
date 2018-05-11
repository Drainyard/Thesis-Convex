#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec4 color;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 normal;
out vec3 posWorld;
out vec4 c;

void main()
{
	gl_Position = P * V * M * vec4(position, 1);

	posWorld  = (M * vec4(position,1)).xyz;
	normal = mat3(transpose(inverse(V * M))) * vertexNormal;

	c = color;
}
