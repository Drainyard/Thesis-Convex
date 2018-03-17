#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 color;
uniform float thickness;
uniform float aspect;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 previous;
layout(location = 2) in vec3 next;
layout(location = 3) in float direction;

out vec4 c;
out vec2 cur;
out vec2 nex;
out vec2 pre;
out vec2 d;

void main()
{
	vec2 aspectVec = vec2(aspect, 1.0);
	mat4 projViewModel = projection * view * model;
	
	//into clip space
	vec4 previousProjected = projViewModel * vec4(previous, 1.0);
	vec4 currentProjected = projViewModel * vec4(position, 1.0);
	vec4 nextProjected = projViewModel * vec4(next, 1.0);


	//into NDC space [-1 .. 1]
	//correct for aspect ratio (screenSize.x / screenSize.y)
	vec2 previousScreen = previousProjected.xy / previousProjected.w * aspectVec;
	vec2 currentScreen = currentProjected.xy / currentProjected.w * aspectVec;
	vec2 nextScreen = nextProjected.xy / nextProjected.w  * aspectVec;

	vec2 dir = vec2(0.0, 0.0);
	if(currentScreen == previousScreen)
	{
		dir = normalize(nextScreen - currentScreen);
	}
	else if(currentScreen == nextScreen)
	{
		dir = normalize(currentScreen - previousScreen);
	}

	//extrude from center & correct aspect ratio
	vec2 n = vec2(-dir.y, dir.y);
	n *= thickness/2.0;
	n.x /= aspect;

	//offset by the direction of this point in the pair (-1 or 1)
	vec4 offset = vec4(n * direction, 0.0, 0.0);
	gl_Position = currentProjected + offset;

	c = color;
	cur = currentScreen;
	nex = nextScreen;
	pre = previousScreen;
	d = dir;
}

