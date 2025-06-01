#version 460 core

in vec3 fragPos;
out vec4 FragColor;

uniform vec3 aCol;
uniform float alpha;

float random (vec2 st);

void main()
{
	FragColor = vec4(aCol * fragPos, alpha);
}