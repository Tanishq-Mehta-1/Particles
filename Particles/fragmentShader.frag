#version 460 core

out vec4 FragColor;

uniform vec3 aCol;
uniform float alpha;

void main()
{
	FragColor = vec4(aCol, alpha) ;	
}