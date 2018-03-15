#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 color;
uniform float thickness;
uniform float aspect;
uniform vec2 direction;
uniform vec3 start;
uniform vec3 end;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 normal;
layout(location = 2) in float miter;

out vec4 c;

void main()
{
	mat4 projViewModel = projection * view * model;
	
	//into clip space
	vec4 currentProjected = projViewModel * vec4(position, 1.0);

	//into NDC space [-1 .. 1]
	vec2 currentScreen = currentProjected.xy / currentProjected.w;

	//correct for aspect ratio (screenSize.x / screenSize.y)
	currentScreen.x *= aspect;

	//extrude from center & correct aspect ratio
	vec2 direction;
	if(miter == -1.0)
	{
		direction = normalize(vec2(start.x - end.x, start.y - end.y));
	}
	else
	{
		direction = normalize(vec2(end.x - start.x, end.y - start.y));
		
	}

	vec2 n = vec2(-direction.y, direction.y);
	n *= thickness/2.0;
	n.x /= aspect;

	//offset by the direction of this point in the pair (-1 or 1)
	vec4 offset = vec4(n * direction, 0.0, 1.0);
	gl_Position = currentProjected + offset;

	c = color;
}
