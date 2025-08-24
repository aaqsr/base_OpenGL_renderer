#version 410 core

out vec4 FragColour;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D theTexture; // The diffuse texture

uniform vec3 viewPos;

void main()
{
    vec3 objectColour = texture(theTexture, TexCoord).rgb;
    float shininess = 32.0;

    vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.7));
    vec3 lightColour = vec3(1.0, 1.0, 1.0); // white light

    // Ambient Lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColour;

    // Diffuse Lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColour;

    // Specular Lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColour;

    vec3 result = (ambient + diffuse) * objectColour + specular;
    FragColour = vec4(result, 1.0);
}
