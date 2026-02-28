#version 460 core
#extension GL_ARB_bindless_texture : require
struct SSBO_data
{
	mat4 transform;
	int mask;
	uint if_draw_instanced;
	uvec2 TexHandle;
	float vertices[20];
	
};
layout(std430, binding = 0) buffer SSBO
{
	SSBO_data objData[50];
};
out VS_OUT
{
	vec2 TexCoord;
	flat uvec2 Tex;
	flat int Mask;
} vs_out;
uint EBO[6] = uint[](0, 1, 2,   0, 2, 3);
void main()
{
	if (objData[gl_InstanceID].if_draw_instanced == 0)
	{
		gl_Position = vec4(2.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		uint localVertexID = EBO[gl_VertexID];
		vec3 VertCoord = vec3(objData[gl_InstanceID].vertices[localVertexID*5], objData[gl_InstanceID].vertices[localVertexID*5 + 1], objData[gl_InstanceID].vertices[localVertexID*5 + 2]);
		gl_Position = objData[gl_InstanceID].transform*vec4(VertCoord, 1.0f);
		vs_out.TexCoord = vec2(objData[gl_InstanceID].vertices[localVertexID*5 + 3], objData[gl_InstanceID].vertices[localVertexID*5 + 4]);
		vs_out.Tex = objData[gl_InstanceID].TexHandle;
		vs_out.Mask = objData[gl_InstanceID].mask;
	}
	
}