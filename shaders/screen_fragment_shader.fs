#version 460 core

out vec4 FragColor;
uniform sampler2D Tex;
in vec2 TexCoord;

void main()
{
	FragColor = texture(Tex, TexCoord);
}
