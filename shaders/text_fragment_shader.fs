#version 460 core

layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;

in vec2 TexCoord;
uniform sampler2D Atlas;
uniform int Mask;
void main()
{
	float alpha = texture(Atlas, TexCoord).r;
	FragColor = vec4(0.3f, 0.6f, 0.9f, alpha);
	FragMask = Mask;
}