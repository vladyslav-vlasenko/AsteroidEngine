#version 460 core
layout(location=0) in vec2 VCoord;
uniform mat4 transform;
void main()
{
	gl_Position = transform * vec4(VCoord, 0.0f, 1.0f);
}