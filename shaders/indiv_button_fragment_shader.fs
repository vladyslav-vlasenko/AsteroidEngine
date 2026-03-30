#version 460 core
layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;

in vec2 TexCoord;
in flat int Type;
uniform sampler2D Tex;
uniform int Mask;
uniform vec2 callPos;
void main()
{
	vec4 color = texture(Tex, TexCoord);
	if(color.x > 0.9f && color.y > 0.9f && color.z > 0.9f)
		discard;
	
	if (Type == 0)
	{
		if(callPos.x != -2.0f && gl_FragCoord.y>callPos.y)
		discard;
		FragColor = color;
	}
	else if (Type == 1)
	{
		FragColor = vec4(0.1f, 0.4f, 0.4f, 1.0f);
	}
	FragMask = Mask;
	
}