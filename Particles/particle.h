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

	//properties
	float radius;
	float mass{ 1.0f };
	float density{ 1.0f };
	float alpha{ 1.0f };
	glm::vec3 colour;
	glm::vec2 position;
	bool hasCollided{ false };

	float restitution_coefficient{ 1.0f };
	glm::vec2 velocity{ 0.0f, 0.0f };
	float pixelsPerMeter{ 300.0f / 9.8f };
	glm::vec2 acceleration{ 0.0f, 0.0f }; 
	//remove pixelsPerMeter for slowmo haha

	Particle(float r, glm::vec2 p, GLFWwindow* window, glm::vec3 col, float e, float alph) {
		radius = r;
		position = p;
		colour = col;
		restitution_coefficient = e;
		mass =  4 /3 * 3.14 * radius * radius * radius * density; //rendered as disks, not spheres
		alpha = alph;

		glfwGetWindowSize(window, &window_width, &window_height);
	}

	void drawCircle(unsigned int VAO, Shader shader, int res) const
	{
		shader.use();
		glBindVertexArray(VAO);

		shader.setVec3("aCol", this->colour);
		shader.setFloat("alpha", this->alpha);

		float width = window_width;
		float height = window_height;
		glm::mat4 projection = glm::ortho(-width / 2, width / 2, -height / 2, height / 2, -1.0f, 1.0f);
		shader.setMat4("projection", projection);
		shader.setMat4("model", modelMatrix);

		//draw call
		glDrawArrays(GL_TRIANGLE_FAN, 0, res + 2);
		glBindVertexArray(0);
	}

	void update(float deltaTime, GLFWwindow* window, bool velocity_colouring, bool collision_colouring)
	{
		handleBoundaryCollision();

		if(velocity_colouring)
			enableVelocityColouring();
		else if (collision_colouring)
			enableCollisionColouring();

		//drag calculations
		glm::vec2 a_drag = glm::vec2(0.0f);
		calculateDrag(a_drag);
		velocity += a_drag * deltaTime;

		float maxVelocity{ 8000.0f }; //doesnt tunnel till 18k, but 12k looks good
		velocity += acceleration * deltaTime;
		velocity = glm::clamp(velocity, -glm::vec2(maxVelocity), glm::vec2(maxVelocity));

		position += velocity * deltaTime;

		//update model Mat
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(position, 0.0f)); //the vector to translate by must be withing -1,1 to stay on screen
		modelMatrix = glm::scale(modelMatrix, 2.0f * radius * glm::vec3(1.0));
	}

	glm::mat4 getModel() const {
		return modelMatrix;
	}

private:

	int window_width;
	int window_height;
	glm::mat4 modelMatrix{1.0f};

	void handleBoundaryCollision()
	{
		float bound_y = window_height / 2;
		float bound_x = window_width / 2;
		float e = 1.0f; //boundary coefficient of restitution

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

	float Green_Speed{ 375.0f  }, Red_Speed{ 750.0f }, White_Speed{ 12000.0f};
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

	void enableCollisionColouring() {
		if (hasCollided)
		{
			colour = glm::vec3(1.0f, 0.0f, 0.0f);
			hasCollided = false;
		}
		else {
			colour.r -= 0.06f;
			colour.b += 0.06f;
		}
	}

	void calculateDrag(glm::vec2 &a_drag) const
	{
		//implementing rudimentary drag 
		float speed = length(velocity) / pixelsPerMeter;
		float fluid_density = 1.0f; //air has 1.0f
		glm::vec2 drag_dir = speed > 0.001f ? -normalize(velocity) : glm::vec2(0.0f);
		a_drag = 0.5f * 0.47f * 3.14f * radius * radius * speed * speed * (1 / mass) * fluid_density * drag_dir; //0.47 is drag coefficient of sphere
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
		glm::vec2 normal = distance == 0 ? glm::vec2(1.0f, 0.0f) : delta / distance;

		float velocity_along_normal = glm::dot(relative_velocity, normal); //relative velocity along normal
		
		if (velocity_along_normal > 0.0f) //the particles are separating
			return;

		//calculate impulse
		float m1 = p1.mass;
		float m2 = p2.mass;
		float e = std::min(p1.restitution_coefficient, p2.restitution_coefficient);
		float j = -(1 + e) * velocity_along_normal / (1/m1 + 1/m2);
		glm::vec2 impulse = j * normal;

		p1.velocity += impulse / m1;
		p2.velocity -= impulse / m2;

		//resolving interpenetration
		//the weights ensure that the heavier mass moves less
		float interpenetration = p1.radius + p2.radius - distance;
		if (interpenetration > 0.0f)
		{
			glm::vec2 offset = (interpenetration / (m1 + m2)) * normal;
			p1.position += offset * m2 ;
			p2.position -= offset * m1 ;
		}

		p1.hasCollided = true;
		p2.hasCollided = true;
	}
}

void handleGravity(Particle& p1, Particle& p2)
{
	glm::vec2 distance = p1.position - p2.position; //already the squared distance

	float softening = 10.0f;
	float r2 = dot(distance, distance) + softening * softening;
	if (r2 > 400.0f * 400.0f || r2 == 0.01)
		return;
	glm::vec2 dir = (r2 == 0) ? glm::vec2(0.0f) : distance / sqrt(r2);

	float m1 = p1.mass;
	float m2 = p2.mass;
	float G_mod = 6.6743 * pow(10,-1) * p1.pixelsPerMeter;

	p1.acceleration -= G_mod * (m2 / r2)  * dir;
	p2.acceleration += G_mod * (m1 / r2) * dir;
}

#endif
