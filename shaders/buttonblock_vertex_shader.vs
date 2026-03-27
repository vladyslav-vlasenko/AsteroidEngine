#version 460 core
layout(location=0) in vec3 VCoord;
layout(location=1) in vec2 TCoord;

out VS_OUT{
	vec2 TexCoord;
	flat int type; 
} vs_out;

vec2 mult_vec(in vec2 a, in vec2 b)
{
	vec2 res = vec2(a.x*b.x, a.y*b.y);
	return res;
}

uniform int Type;
uniform mat4 transform;
uniform vec2 scale_vec;
void main()
{
	vs_out.TexCoord = TCoord;
	vs_out.type = Type;
	if(Type == 0)
		gl_Position = transform*vec4(VCoord, 1.0f);
	if(Type == 1)
		gl_Position = transform*vec4(mult_vec(VCoord.xy, scale_vec), VCoord.z, 1.0f);

}