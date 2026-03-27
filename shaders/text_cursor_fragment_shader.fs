#version 460 core

layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;
uniform float intensity;
void main()
{

	FragColor = vec4(1.0f, 0.0f, 0.0f, intensity/255);
	FragMask = 39;
}