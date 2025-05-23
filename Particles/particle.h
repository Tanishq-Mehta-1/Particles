#ifndef PARTICLES_H
#define PARTICLES_H

#include <iostream>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <shader.h>

class Particle 
{
public:

	float radius;
	glm::vec2 center;
	glm::vec2 position; 

	glm::vec2 velocity{ 0.0f, -0.0001f };
	glm::vec2 acceleration{ 0.0f };

	Particle(float r, glm::vec2 c, glm::vec2 p) {
		radius = r;
		center = c;
		position = p;
	}

	void drawCircle(unsigned int VAO, Shader shader, int res) const
	{
		shader.use();
		glBindVertexArray(VAO);

		glm::mat4 translation = glm::mat4(1.0f);
		translation = glm::translate(translation, glm::vec3(position, 0.0f)); //the vector to translate by must be withing -1,1 to stay on screen
		translation = glm::scale(translation, 2.0f * radius * glm::vec3(1.0));
		shader.setMat4("translation", translation);
		glDrawArrays(GL_TRIANGLE_FAN, 0, res + 2);

		glBindVertexArray(0);
	}

	void update()
	{
		velocity += acceleration;
		position += velocity;
	}




};

#endif
