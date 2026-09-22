#version 300 es

precision mediump float;

in vec3 aCol;
//in float alpha;

in vec3 fragPos;
out vec4 FragColor;

void main()
{
	FragColor = vec4(aCol, 1.0f);
}