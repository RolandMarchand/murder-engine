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

struct LightColor {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct LightFalloff {
	float constant;
	float linear;
	float quad;
};

struct Sunlight {
	LightColor color;
	vec3 dir;
};

struct LightPoint {
	LightColor color;

	vec3 position;
	LightFalloff falloff;
};

struct Spotlight {
	LightColor color;

	// Cone
	vec3 position;
	vec3 dir;
	float cutoff;
	float outerCutoff;
	
	LightFalloff falloff;
};

uniform Sunlight sunlight;
uniform LightPoint lightPoint;
uniform Spotlight spotlight;
uniform Material material;
uniform vec3 viewPos;

LightColor calculateLightColor(LightColor _lightColor, vec3 lightDir)
{
	// Ambient light
	vec3 ambient = _lightColor.ambient *
		vec3(texture(material.diffuse, TexCoord));

	// Diffused light
	vec3 norm = normalize(Normal);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * _lightColor.diffuse *
		vec3(texture(material.diffuse, TexCoord));

	// Specular light. Very weird, I need to reverse the camera position
	// (viewPos), maybe there is an issue with the coordinate system.
	vec3 viewDir = normalize(-viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f),
			 material.shininess);
	vec3 specular = spec * _lightColor.specular *
		vec3(texture(material.specular, TexCoord));

	return LightColor(ambient, diffuse, specular);
}

float calculateFalloff(LightFalloff _lightFalloff, float distance)
{
	return 1.0f / (_lightFalloff.constant + _lightFalloff.linear *
		       distance + _lightFalloff.quad * (distance * distance));
}

vec3 getSunlight()
{
	vec3 lightDir = normalize(-sunlight.dir);
	LightColor l = calculateLightColor(sunlight.color, lightDir);
	return l.ambient + l.diffuse + l.specular;
}

vec3 getLightPoint()
{
	vec3 lightDir = normalize(lightPoint.position - FragPos);
	LightColor l = calculateLightColor(lightPoint.color, lightDir);

	// Attenuation
	float distance = length(lightPoint.position - FragPos);
	float attenuation = calculateFalloff(lightPoint.falloff, distance);
	l.ambient *= attenuation;
	l.diffuse *= attenuation;
	l.specular *= attenuation;

	return l.ambient + l.diffuse + l.specular;
}

vec3 getSpotlight()
{
	vec3 lightDir = normalize(-spotlight.position - FragPos);
	LightColor l = calculateLightColor(spotlight.color, lightDir);

	// Cone

	// Another occurrence of the weird camera position negation.
	float theta = dot(lightDir, normalize(-spotlight.dir));
	float epsilon   = spotlight.cutoff - spotlight.outerCutoff;
	float intensity = clamp((theta - spotlight.outerCutoff) /
				epsilon, 0.0f, 1.0f);
	float factor = mix(intensity, 1.0f, step(spotlight.cutoff, theta));
	l.ambient *= factor;
	l.diffuse *= factor;
	l.specular *= factor;

	// Attenuation
	float distance = length(-spotlight.position - FragPos);
	float attenuation = calculateFalloff(spotlight.falloff, distance);
	l.ambient *= attenuation;
	l.diffuse *= attenuation;
	l.specular *= attenuation;

	return l.ambient + l.diffuse + l.specular;
}

void main()
{
	vec3 result = getSunlight();
	vec3 point = getLightPoint();
	vec3 spot = getSpotlight();
	FragColor = vec4(point + result + spot, 1.0f);
}
