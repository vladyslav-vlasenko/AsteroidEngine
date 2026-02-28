#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;
in VS_OUT
{
	vec2 TexCoord;
	flat uvec2 Tex;
	flat int Mask;
} fs_in;
void main()
{
	FragColor = texture(sampler2D(fs_in.Tex), fs_in.TexCoord);
	
	if(FragColor.x > 0.9f && FragColor.y > 0.9f && FragColor.z > 0.9f)
		discard;
	FragMask = fs_in.Mask;
	
}