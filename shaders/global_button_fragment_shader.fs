#version 460 core
#extension GL_ARB_bindless_texture : require
layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;

in flat int Mask;
in flat uvec2 TexHandler;
in flat vec2 callPos;
in vec2 TexCoord;

void main()
{
	if (callPos.x == -2.0f)
	{
		vec4 color = texture(sampler2D(TexHandler), TexCoord);
		if (color.x>0.95f && color.y>0.95f && color.z>0.95f)
			discard;
		FragColor = color;
		FragMask = Mask;
	}
	else
	{
		vec4 color = texture(sampler2D(TexHandler), TexCoord);
		if (color.x>0.95f && color.y>0.95f && color.z>0.95f)
			discard;
		if (gl_FragCoord.y > callPos.y)
			discard;
		FragColor = color;
		FragMask = Mask;
	}
}