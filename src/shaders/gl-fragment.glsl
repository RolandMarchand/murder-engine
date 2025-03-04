#version 330 core
out vec4 FragColor;
  
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform sampler2D laz;
uniform sampler2D blue;
uniform Light light;
uniform Material material;
uniform vec3 viewPos;

void main()
{
	// Base color
	vec3 objectColor = mix(texture(laz, TexCoord),
			       texture(blue, TexCoord), 0.5f).xyz;

	// Ambient light
	vec3 ambient = light.ambient * material.ambient;

	// Diffused light
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * light.diffuse * material.diffuse;

	// Specular light. Very weird, I need to reverse the camera position
	// (viewPos), maybe there is an issue with the coordinate system.
	vec3 viewDir = normalize(-viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f),
			 material.shininess);
	vec3 specular = spec * light.specular * material.specular;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0f);
}
