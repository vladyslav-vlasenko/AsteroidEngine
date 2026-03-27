#version 460 core

layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;

in flat int Type;
in vec2 TexCoord;

uniform sampler2D Tex;
uniform float callPos_y;
uniform int Mask;

void main()
{
	vec4 color = texture(Tex, TexCoord);
	if (Type == 0)
	{
		if(color.x>=0.95f && color.y>=0.95f && color.z>=0.95f)
			discard;
		else if(gl_FragCoord.y>callPos_y)
			discard;
		FragColor = color;
	}
	else if (Type == 1)
	{
		if(color.x>=0.95f && color.y>=0.95f && color.z>=0.95f)
			discard;
		FragColor = vec4(0.4f, 0.4f, 0.4f, 1.0f);
	}
	FragMask = Mask;
}
