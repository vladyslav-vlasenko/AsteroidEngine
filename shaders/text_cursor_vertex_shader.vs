#version 460 core
layout(location=0) in vec2 VCoord;
uniform mat4 transform;
uniform int type;
out flat int Type;
void main()
{
	Type = type;
	if(type == 0)
	{
		gl_Position = transform * vec4(VCoord, 0.0f, 1.0f);
	}
	if(type == 1)
	{
		gl_Position = vec4(VCoord, 0.0f, 1.0f);
	}
}