#version 460 core
layout(location=0) in vec3 VCoord;
layout(location=1) in vec2 TCoord;

out VS_OUT{
	vec2 TexCoord;
	flat int type; 
} vs_out;
uniform int Type;
uniform mat4 transform;

void main()
{
	vs_out.TexCoord = TCoord;
	vs_out.type = Type;
	if(Type == 0)
		gl_Position = transform*vec4(VCoord, 1.0f);
	if(Type == 1)
		gl_Position = transform*vec4(1.1f*VCoord, 1.0f);

}