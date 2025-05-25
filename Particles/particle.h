#ifndef PARTICLES_H
#define PARTICLES_H

#include <iostream>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <shader.h>

//void handleParticleCollisions(Particle& p1, Particle& p2);
void displayVec2(glm::vec2 vector);
void displayVec3(glm::vec3 vector);

class Particle
{
public:

	float radius;
	float mass{ 1.0f };
	float density{ 1.0f };
	int window_width;
	int window_height;
	glm::vec3 colour;
	glm::vec2 position;

	float restitution_coefficient{ 0.7f };
	glm::vec2 velocity{ 0.0f, 0.0f };
	float pixelsPerMeter{ 300.0f / 9.8f };
	glm::vec2 acceleration{ 0.0f, -9.8f * pixelsPerMeter }; 
	//remove pixelsPerMeter for slowmo haha
	Particle(float r, glm::vec2 p, GLFWwindow* window, glm::vec3 col) {
		radius = r;
		position = p;
		colour = col;
		mass = 4.0f / 3.0f * 3.14 * radius * radius * radius * density;

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
		glm::mat4 projection = glm::ortho(-width / 2, width / 2, -height / 2, height / 2, -1.0f, 1.0f);
		shader.setMat4("projection", projection);

		//draw call
		glDrawArrays(GL_TRIANGLE_FAN, 0, res + 2);

		glBindVertexArray(0);
	}

	void update(float deltaTime, GLFWwindow* window)
	{
		handleBoundaryCollision(window);
		enableVelocityColouring();

		float maxVelocity{ 12000.0f }; //doesnt tunnel till 18k, but 12k looks good
		velocity += acceleration * deltaTime;
		velocity = glm::clamp(velocity, -glm::vec2(maxVelocity), glm::vec2(maxVelocity));
		position += velocity * deltaTime;
		acceleration.x =  200 * sin(glfwGetTime());
	}

private:

	void handleBoundaryCollision(GLFWwindow* window)
	{
		float bound_y = window_height / 2;
		float bound_x = window_width / 2;
		float e = 0.7f; //boundary coefficient of restitution

		//very important to clamp the positions, else the bounces are not completely elastic
		if (position.y - radius <= -bound_y)
		{
			velocity.y *= -e;
			position.y = -bound_y + radius;
		}
		else if (position.y + radius >= bound_y)
		{
			velocity.y *= -e;
			position.y = bound_y - radius;
		}

		if (position.x - radius <= -bound_x)
		{
			velocity.x *= -e;
			position.x = -bound_x + radius;
		}
		else if (position.x + radius >= bound_x)
		{
			velocity.x *= -e;
			position.x = bound_x - radius;
		}
	}

	float Green_Speed{ 500.0f  }, Red_Speed{ 1000.0f  }, White_Speed{ 12000.0f};
	void enableVelocityColouring()
	{
		float speed = glm::length(velocity);
		//W = 12K, R = 8K, G = 4K, B = 0K, interpolate between these 
		if (speed < Green_Speed)
			colour = glm::vec3(0.0f, 1.0f, -1.0f) * (speed / Green_Speed) + glm::vec3(0.0f, 0.0f, 1.0f);
		else if (speed < Red_Speed)//speed >= 4k and < 8k
			colour = glm::vec3(1.0f, -1.0f, 0.0f) * ((speed - Green_Speed) / Red_Speed - Green_Speed) + glm::vec3(0.0f, 1.0f, 0.0f);
		else //speed >= 8k and <= 12k
			colour = glm::vec3(0.0f, 1.0f, 1.0f) * ((speed - Red_Speed) / White_Speed - Red_Speed) + glm::vec3(1.0f, 0.0f, 0.0f);
	}
};

//utility functions
void displayVec2(glm::vec2 vector)
{
	std::cout << vector.x << ' ' << vector.y << '\n';
}

void displayVec3(glm::vec3 vector)
{
	std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << '\n';
}

void handleParticleCollisions(Particle& p1, Particle& p2)
{
	glm::vec2 delta = p1.position - p2.position;
	float distance = length(delta);

	//detect a bit before actual collision to prevent jittering
	if (distance <= p1.radius + p2.radius - 0.0001f) {

		//normalize(vec) = vec / length(vec), which is NaN for 0 vectors
		glm::vec2 relative_velocity = p1.velocity - p2.velocity;
		glm::vec2 normal = distance == 0 ? glm::normalize(relative_velocity) : glm::normalize(delta);

		float velocity_along_normal = glm::dot(relative_velocity, normal); //relative velocity along normal
		
		if (velocity_along_normal > 0.0f) //the particles are separating
			return;

		//calculate impulse
		float m1 = p1.mass;
		float m2 = p2.mass;
		float e = std::min(p1.restitution_coefficient, p2.restitution_coefficient);
		float j = -(1 + e) * velocity_along_normal / (1/m1 + 1/m2);
		glm::vec2 impulse = j * normal;
		//glm::vec2 impulse = -velocity_along_normal * normal;

		p1.velocity += impulse / m1;
		p2.velocity -= impulse / m2;
		//p1.velocity += impulse;
		//p2.velocity -= impulse;

		//resolving interpenetration
		//the weights ensure that the heavier mass moves less
		float interpenetration = p1.radius + p2.radius - distance;
		if (interpenetration > 0.0f)
		{
			glm::vec2 offset = (interpenetration / (m1 + m2)) * normal;
			p1.position += offset * m2;
			p2.position -= offset * m1;
		}
	}
}

#endif
