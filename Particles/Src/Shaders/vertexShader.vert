#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in mat4 model;
layout (location = 5) in vec3 Color;

uniform mat4 projection;
 
//uniform vec3 Col; //instanced array of MVP
//uniform float alpha_in; //instanced array of MVP

out vec3 fragPos;
out vec3 aCol;
//out float alpha;


void main()
{
	gl_Position =  projection * model * vec4(aPos, 0.0f, 1.0f);
	fragPos = (gl_Position).xyz;

	aCol = Color;
}
