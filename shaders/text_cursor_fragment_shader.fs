#version 460 core

layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;
uniform float intensity;
in flat int Type;
uniform int Mask;

void main()
{
	
	if (Type == 0)
	{
		FragColor = vec4(1.0f, 0.0f, 0.0f, intensity/255);
		FragMask = Mask;
	}
	if (Type == 1)
	{
		FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		FragMask = Mask;
	}
}