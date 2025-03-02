#version 330 core
out vec4 FragColor;
  
in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D laz;
uniform sampler2D blue;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	float ambientStrength = 0.2f;
	float specularStrength = 1.0f;

	// Base color
	vec3 objectColor = mix(texture(laz, TexCoord),
			       texture(blue, TexCoord), 0.5f).xyz;

	// Ambient light
	vec3 ambient = ambientStrength * lightColor;

	// Diffused light
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	// Specular light. Very weird, I need to reverse the camera position
	// (viewPos), maybe there is an issue with the coordinate system.
	vec3 viewDir = normalize(-viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0f);
}
