#version 460 core
struct Characters
{
	vec2 lu_uv;
	vec2 rl_uv;
	vec2 lu_st;
	vec2 rl_st;
};

layout(std430, binding=2) buffer Character
{
	Characters characters[256];
};

layout(std140, binding=0) uniform UBO
{
	float text_size;
	float padding1;
	vec2 padding2;
	mat4 transforms[128];
	ivec4 character_list[32];
};
vec2 vec2_mult(in vec2 a, in vec2 b)
{
	vec2 res = vec2(a.x*b.x, a.y*b.y);
	return res;
}
uint ebo[6] = uint[](0, 1, 2,   0, 2, 3);
out vec2 TexCoord;
uniform vec2 scale;
void main()
{
	if (character_list[gl_InstanceID/4][gl_InstanceID%4] != -1)
	{
		vec2 lu_st = characters[character_list[gl_InstanceID/4][gl_InstanceID%4]].lu_st;
		vec2 lu_uv = characters[character_list[gl_InstanceID/4][gl_InstanceID%4]].lu_uv;
		vec2 rl_st = characters[character_list[gl_InstanceID/4][gl_InstanceID%4]].rl_st;
		vec2 rl_uv = characters[character_list[gl_InstanceID/4][gl_InstanceID%4]].rl_uv;
		vec2 vertex;
		switch (ebo[gl_VertexID])
		{
			case 0:
			{
				vertex = vec2(text_size * lu_st.x, text_size * rl_st.y);
				TexCoord = vec2(lu_uv.x, rl_uv.y);
				break;
			}
			case 1:
			{
				vertex = vec2(text_size * lu_st.x, text_size * lu_st.y);
				TexCoord = vec2(lu_uv.x, lu_uv.y);
				break;
			}
			case 2:
			{
				vertex = vec2(text_size * rl_st.x, text_size * lu_st.y);
				TexCoord = vec2(rl_uv.x, lu_uv.y);
				break;
			}
			case 3:
			{
				vertex = vec2(text_size * rl_st.x, text_size * rl_st.y);
				TexCoord = vec2(rl_uv.x, rl_uv.y);
				break;
			}
		}
		gl_Position = transforms[gl_InstanceID] * vec4(vec2_mult(vertex, scale), 0.0f, 1.0f);
	}
	else
		gl_Position = vec4(2.0f, 0.0f, 0.0f, 1.0f);
}