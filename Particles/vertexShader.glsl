#version 460 core

layout (location = 0) in vec2 aPos;

uniform mat4 translation;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main()
{
	gl_Position = translation * vec4(aPos, 0.0f, 1.0f);
	FragPos =  vec3(translation * vec4(aPos, 0.0f, 1.0f));
}
