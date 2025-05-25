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
	glm::vec3 colour;
	glm::vec2 position; 

	float restitution_coefficient{ 0.95f };
	glm::vec2 velocity{ 0.0f, 0.0f };
	float pixelsPerMeter{ 500.0f / 9.8f };
	glm::vec2 acceleration{ 0.0f, -9.8f * pixelsPerMeter };

	Particle(float r, glm::vec2 p, GLFWwindow* window, glm::vec3 col) {
		radius = r;
		position = p;
		colour = col;

		glfwGetWindowSize(window, &window_width, &window_height);
	}

	void drawCircle(unsigned int VAO, Shader shader, int res) const
	{
		shader.use();
		glBindVertexArray(VAO);

		shader.setVec3("aCol", colour);

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

		float maxVelocity{ 12000.0f }; //doesnt tunnel till 18k, but 12k looks good
		velocity += acceleration * deltaTime;
		velocity = glm::clamp(velocity, -glm::vec2(maxVelocity), glm::vec2(maxVelocity));

		position += velocity * deltaTime;
	}

	private:
		
		void handleBoundaryCollision(GLFWwindow* window)
		{
			float bound_y = window_height / 2;
			float bound_x = window_width / 2;

			//very important to clamp the positions, else the bounces are not completely elastic
			if (position.y - radius <= -bound_y)
			{
				velocity.y *= -restitution_coefficient;
				position.y = -bound_y + radius;
			}
			else if (position.y + radius >= bound_y) 
			{
				velocity.y *= -restitution_coefficient;
				position.y = bound_y - radius;
			}

			if (position.x - radius <= -bound_x)
			{
				velocity.x *= -restitution_coefficient;
				position.x = -bound_x + radius;
			}
			else if (position.x + radius >= bound_x)
			{
				velocity.x *= -restitution_coefficient;
				position.x = bound_x - radius;
			}
		}

		void displayVec2(glm::vec2 vector) const
		{
			std::cout << vector.x << ' ' << vector.y << '\n';
		}

		void displayVec3(glm::vec3 vector) const
		{
			std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << '\n';
		}


};

#endif
