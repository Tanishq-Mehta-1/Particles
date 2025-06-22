#version 460 core

layout (location = 0) in vec2 aPos;
layout (location  = 1) in mat4 model;

//uniform mat4 model;
uniform mat4 projection;
out vec3 fragPos;

void main()
{
	gl_Position = projection * model * vec4(aPos, 0.0f, 1.0f);
	fragPos = (gl_Position).xyz;
}
