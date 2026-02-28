#version 460 core

layout(location=0) out vec4 FragColor;
layout(location=1) out int FragMask;

in VS_OUT
{
	vec2 TexCoord;
	flat int type;
} fs_in;
uniform int callPosY;
uniform sampler2D Tex;
uniform int Mask;

void main()
{
	if(fs_in.type == 0)
	{
		FragColor = texture(Tex, fs_in.TexCoord);
		if ((FragColor.xyz == vec3(0.0f) || gl_FragCoord.y>callPosY))
			discard;

	}
	else if(fs_in.type == 1)
	{
		FragColor = vec4(0.9f, 0.4f, 0.4f, 1.0f);
		
	}
	FragMask = Mask;
}