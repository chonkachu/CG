#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    vec3 emission;    
    vec3 diffuseReflection;
    vec3 specularReflection;
    float shininess;
    bool isLightSource;
    bool isActive;
    
    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
    vec3 direction;
}; 

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 color;     

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_LIGHTS 10
uniform int numLights;
uniform int numSpotLights;
uniform PointLight pointLights[MAX_LIGHTS];
uniform SpotLight spotLights[MAX_LIGHTS];
uniform DirLight dirLight;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    if (material.isLightSource) {
        vec3 texColor = vec3(texture(material.diffuse, TexCoords));
        if (material.isActive) {
            FragColor = vec4(texColor * material.emission, 1.0);
        }
        else {
            FragColor = vec4(texColor * 0.1, 1.0);
        }
        return;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for(int i = 0; i < numLights; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    for(int i = 0; i < numSpotLights; i++) {
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
    }

    FragColor = vec4(result, 1.0);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    
    float diff = max(dot(normal, lightDir), 0.0);

    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    
    vec3 texColor = vec3(texture(material.diffuse, TexCoords));
    vec3 ambient = light.ambient * texColor * material.diffuseReflection;
    vec3 diffuse = light.diffuse * diff * material.diffuseReflection;
    vec3 specular = light.specular * spec * material.specularReflection;

    return (ambient + diffuse + specular);
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    
    float diff = max(dot(normal, lightDir), 0.0);

    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
            light.quadratic * (distance * distance));

    
    vec3 texColor = vec3(texture(material.diffuse, TexCoords));
    vec3 ambient = light.ambient * texColor * material.diffuseReflection;
    vec3 diffuse = light.diffuse * diff * material.diffuseReflection;
    vec3 specular = light.specular * spec * material.specularReflection;

    
    vec3 colored = (ambient + diffuse + specular) * light.color;
    return colored * attenuation;
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);

    
    float diff = max(dot(normal, lightDir), 0.0);

    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
            light.quadratic * (distance * distance));

    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    
    vec3 texColor = vec3(texture(material.diffuse, TexCoords));
    vec3 ambient = light.ambient * texColor * material.diffuseReflection;
    vec3 diffuse = light.diffuse * diff * material.diffuseReflection;
    vec3 specular = light.specular * spec * material.specularReflection;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular) * light.color;
}
