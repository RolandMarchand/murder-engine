#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D laz;
uniform sampler2D blue;

void main()
{
	FragColor = mix(texture(laz, TexCoord), texture(blue, TexCoord), 0.5f);
}
