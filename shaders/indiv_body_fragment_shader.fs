#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;
in vec2 TexCoord;
uniform uvec2 TexHandle;
uniform int mask;
in flat int Type;
void main()
{

	vec4 color = texture(sampler2D(TexHandle), TexCoord);
	if(color.x>0.9f && color.y>0.9f && color.z>0.9f)
		discard;
	if (Type == 0)
	{
		FragColor = color;
	}
	else if (Type == 1)
	{
		FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	FragMask = mask;
}