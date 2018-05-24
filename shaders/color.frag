#version 330 core

out vec4 color;

struct DirLight 
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight 
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

in vec3 normal;
in vec3 posWorld;
in vec4 c;

uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform vec3 diffuseColor;
uniform vec3 viewPos;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
    // combine results
    vec3 ambient = light.ambient * c.xyz;
    vec3 diffuse = light.diffuse * diff * c.xyz;
    vec3 specular = light.specular * spec * c.xyz;
    return (ambient + diffuse);
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * c.xyz;
    vec3 diffuse = light.diffuse * diff * c.xyz;
    vec3 specular = light.specular * spec * c.xyz;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

void main()
{
    // properties
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - posWorld);
    
    vec3 ambient = vec3(0.3, 0.3, 0.3);
    vec3 result = ambient * c.xyz;
    result += calcDirLight(dirLight, norm, viewDir);
    //result += calcSpotLight(spotLight, norm, posWorld, viewDir);
    
    color = vec4(result, 1.0);
}