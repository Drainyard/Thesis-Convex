#version 330 core

in vec2 UV;
in vec3 normal;
in vec3 posWorld;
in vec3 eyeView;
in vec3 lightDir;

out vec4 color;

uniform sampler2D textureSampler;
uniform vec3 lightPosWorld;
uniform vec3 lightColor;
uniform float lightPower;
uniform vec3 specularColor;
uniform float alpha;

void main()
{
	vec3 n = normalize(normal);

	vec3 l = normalize(lightDir);

	vec3 E = normalize(eyeView);

	vec3 R = reflect(-l,n);

	float cosAlpha = clamp( dot( E,R ), 0,1 );

	float cosTheta = clamp( dot( n,l ), 0,1 );
	float distance = length(lightPosWorld - posWorld);
	vec3 ambientColor = vec3(0.1, 0.1, 0.1) * texture(textureSampler, UV).rgb;
	color.rgb = ambientColor + texture(textureSampler, UV).rgb * lightPower * lightColor * cosTheta / (distance*distance) + specularColor * lightColor * pow(cosAlpha, 5) / (distance*distance);
	color.a = alpha;
}

