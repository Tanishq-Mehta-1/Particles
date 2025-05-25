#version 460 core

in vec3 FragPos;

out vec4 FragColor;

uniform vec3 aCol;

void main()
{
	FragColor = vec4( aCol, 1.0f );
}