#version 460 core

layout (location = 0) in vec2 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 col;
uniform float alp;

out vec3 fragPos;

out vec3 aCol;
out float alpha;

void main()
{
	gl_Position = projection * model * vec4(aPos, 0.0f, 1.0f);
	fragPos = (gl_Position).xyz;

	aCol = col;
	alpha = alp;
}