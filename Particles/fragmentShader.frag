#version 460 core

in vec3 fragPos;
out vec4 FragColor;

uniform vec3 aCol;
uniform float alpha;

void main()
{
	FragColor = vec4(1.0f);
}