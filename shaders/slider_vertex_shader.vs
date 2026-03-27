#version 460 core

vec2 mult_vec(in vec2 a, in vec2 b)
{
	vec2 res = vec2(a.x*b.x, a.y*b.y);
	return res;
}

layout(location=0) in vec3 VCoord;
layout(location=1) in vec2 TCoord;
out flat int Type;
out vec2 TexCoord;
uniform int type;
uniform mat4 transform;
uniform vec2 scale_vec;
void main()
{
	Type = type;
	TexCoord = TCoord;
	if (type == 0)
	{
		gl_Position = transform * vec4(VCoord, 1.0f);
	}
	else if(type == 1)
	{
		gl_Position = transform * vec4(mult_vec(VCoord.xy, scale_vec), VCoord.z, 1.0f);
	}
}
