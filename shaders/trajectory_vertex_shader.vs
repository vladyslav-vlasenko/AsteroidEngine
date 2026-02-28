#version 460 core

layout(location=0) in vec2 PCoord;

uniform float scale_param; 
void main()
{
	gl_Position = vec4(PCoord.x, scale_param*PCoord.y, 0.0f, 1.0f);
	gl_PointSize = 5.0f;
}