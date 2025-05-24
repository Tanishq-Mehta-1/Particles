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
	int window_width;
	int window_height;
	glm::vec2 position; 

	float restitution_coefficient{ 1.1f }; //doesnt work for very low restitution coefficients
	glm::vec2 velocity{ 0.0f, 0.0f };
	glm::vec2 acceleration{ 0.0f , -100.0f};

	Particle(float r, glm::vec2 p, GLFWwindow* window) {
		radius = r;
		position = p;

		glfwGetWindowSize(window, &window_width, &window_height);
	}

	void drawCircle(unsigned int VAO, Shader shader, int res) const
	{
		shader.use();
		glBindVertexArray(VAO);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(position, 0.0f)); //the vector to translate by must be withing -1,1 to stay on screen
		model = glm::scale(model, 2.0f * radius * glm::vec3(1.0));
		shader.setMat4("model", model);
		
		float width = window_width;
		float height = window_height;
		glm::mat4 projection = glm::ortho(-width/2, width / 2, -height / 2, height / 2, -1.0f, 1.0f);
		shader.setMat4("projection", projection);

		//draw call
		glDrawArrays(GL_TRIANGLE_FAN, 0, res + 2);

		glBindVertexArray(0);
	}

	void update(float deltaTime, GLFWwindow* window)
	{
		handleBoundaryCollision(window);
		displayStats();

		velocity += acceleration * deltaTime;
		position += velocity * deltaTime;
	}

	private:

		void handleBoundaryCollision(GLFWwindow* window)
		{
			if (position.y - radius <= -400 || position.y + radius >= 400)
				velocity *= -restitution_coefficient;
		}

		void displayVec2(glm::vec2 vector) const
		{
			std::cout << vector.x << ' ' << vector.y << '\n';
		}

		float highest_pos{position.y};
		void displayStats() 
		{
			highest_pos = highest_pos < position.y ? position.y : highest_pos;

			std::cout << "\rhighest: " << (int)highest_pos;
			std::cout.flush();
		}

};

#endif
