#version 460 core
#extension GL_ARB_bindless_texture : require
struct BUTTON_SSBO
{
	mat4 transform;
	vec2 callPos;
	int mask;
	int if_global;
	uvec2 TexHandler;
	int if_draw;
};
layout(std430, binding=1) buffer SSBO
{
	BUTTON_SSBO buttons[50];
};
uint ebo[6] = uint[](0, 1, 2,   0, 2, 3);
float vertices[12] = float[]( -1.0f, -1.0f, 0.0f, 
							  -1.0f,  1.0f, 0.0f,
							   1.0f,  1.0f, 0.0f,
							   1.0f, -1.0f, 0.0f);
float uvs[8] = float[]( 0.0f, 0.0f,
						0.0f, 1.0f,
						1.0f, 1.0f,
						1.0f, 0.0f);
out flat int Mask;
out flat uvec2 TexHandler;
out flat vec2 callPos;
out vec2 TexCoord;
void main()
{
	int if_global = buttons[gl_InstanceID].if_global;
	int if_draw = buttons[gl_InstanceID].if_draw;
	if (if_global == 0 || if_draw == 0)
	{
		gl_Position = vec4(2.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (if_global == 1 && if_draw == 1)
	{
		uint index = ebo[gl_VertexID];
		vec2 pos[4] = vec2[](vec2(-0.5, -0.5), vec2(-0.5, 0.5), vec2(0.5, 0.5), vec2(0.5, -0.5));
		vec3 VertCoord = vec3(vertices[3*index], vertices[3*index+1], vertices[3*index+2]);
		TexCoord = vec2(uvs[2*index], uvs[2*index+1]);
		Mask = buttons[gl_InstanceID].mask;
		TexHandler = buttons[gl_InstanceID].TexHandler;
		callPos = buttons[gl_InstanceID].callPos;
		gl_Position = buttons[gl_InstanceID].transform*vec4(VertCoord, 1.0f);
	}
}
