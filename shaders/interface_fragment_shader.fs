//vertex shader coinsides with screen vertex shader
#version 460 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texColor;
uniform isampler2D texMask;
void main()
{
	float mask = texture(texMask, TexCoord).x;
	if(mask==0) discard;
	FragColor = texture(texColor, TexCoord);
}