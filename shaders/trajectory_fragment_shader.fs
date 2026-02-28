#version 460 core

layout(location=0) out vec4 FragColor;
layout(location=1) out int MaskColor;
uniform float intense;
void main()
{
	FragColor = vec4(intense, intense, intense, 1.0f);
	MaskColor = 0;
}