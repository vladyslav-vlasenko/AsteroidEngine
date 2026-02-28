#version 460 core
#extension GL_ARB_bindless_texture : require



layout(location=0) in vec3 VCoord;
layout(location=1) in vec2 TCoord;
out vec2 TexCoord;
uniform mat4 transform;
uniform int type;
out flat int Type;
void main()
{
	TexCoord = TCoord;
	Type = type;
	if (type == 0)
		gl_Position = transform*vec4(VCoord, 1.0f);
	else if (type == 1)
		gl_Position = transform*vec4(1.2f*VCoord, 1.0f);
}