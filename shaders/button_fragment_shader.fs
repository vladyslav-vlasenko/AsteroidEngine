#version 460 core
layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;

in vec2 TexCoord;
in flat int Type;
uniform sampler2D Tex;
uniform int Mask;

void main()
{
	vec4 color = texture(Tex, TexCoord);
	if(color.x > 0.9f && color.y > 0.9f && color.z > 0.9f)
		discard;
	if (Type == 0)
	{
		FragColor = color;
	}
	else if (Type == 1)
	{
		FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	}
	FragMask = Mask;
}