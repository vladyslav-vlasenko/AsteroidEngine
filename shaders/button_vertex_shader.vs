#version 460 core
layout(location=0) in vec3 VCoord;
layout(location=1) in vec2 TCoord;

vec2 vec2_mult(in vec2 v1, in vec2 v2)
{
	vec2 res = vec2(v1.x*v2.x, v1.y*v2.y);
	return res;
}

uniform mat4 transform;
out vec2 TexCoord;
uniform int type;
uniform vec2 scale_vec;
out flat int Type;
void main()
{
	TexCoord = TCoord;
	Type = type;
	if (Type == 0)
	{
		gl_Position = transform*vec4(VCoord, 1.0f);
	}
	else if (Type == 1)
	{
		gl_Position = transform*vec4(vec2_mult(VCoord.xy, scale_vec.xy), VCoord.z, 1.0f);
	}
}