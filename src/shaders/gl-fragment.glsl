#version 330 core
out vec4 FragColor;
  
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;
uniform Material material;
uniform vec3 viewPos;

void main()
{
	// Ambient light
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

	// Diffused light
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * light.diffuse *
		vec3(texture(material.diffuse, TexCoord));

	// Specular light. Very weird, I need to reverse the camera position
	// (viewPos), maybe there is an issue with the coordinate system.
	vec3 viewDir = normalize(-viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f),
			 material.shininess);
	vec3 specular = spec * light.specular *
		vec3(texture(material.specular, TexCoord));

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0f);
}
