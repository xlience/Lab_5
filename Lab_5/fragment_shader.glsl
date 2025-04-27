#version 460 core

out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Light Light_1;
uniform Material Mat_1;

void main() {
    // Ambient
    vec3 ambient = Light_1.ambient * Mat_1.ambient;
    
    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(Light_1.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = Light_1.diffuse * (diff * Mat_1.diffuse);
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), Mat_1.shininess);
    vec3 specular = Light_1.specular * (spec * Mat_1.specular);
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
