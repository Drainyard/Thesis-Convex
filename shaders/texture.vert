#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 lightPosWorld;

out vec2 UV;
out vec3 normal;
out vec3 posWorld;
out vec3 eyeView;
out vec3 lightDir;

void main()
{
	gl_Position = P * V * M * vec4(position, 1);
	UV = vertexUV;

	posWorld  = (M * vec4(position,1)).xyz;

	vec3 viewPos = ( V * M * vec4(position,1)).xyz;
	eyeView = vec3(0,0,0) - viewPos;

	vec3 lightPosView = ( V * vec4(lightPosWorld, 1)).xyz;
	lightDir = lightPosView + eyeView;

	normal = mat3(V * transpose(inverse(M))) * vertexNormal;
}


