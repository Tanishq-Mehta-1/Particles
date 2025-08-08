#ifndef PHYSICS_HANDLER_H
#define PHYSICS_HANDLER_H

#include "Headers/particle.h"
#include <vector>

class PhysicsHandler {

public:
	std::vector<Particle> points;

private:
	int window_width;
	int window_height;
	glm::mat4 modelMatrix{ 1.0f };

	void handleBoundaryCollision(Particle& p)
	{
		float bound_y = window_height / 2;
		float bound_x = window_width / 2;
		float e = 1.0f; //boundary coefficient of restitution

		//very important to clamp the positions, else the bounces are not completely elastic
		if (p.position.y - p.radius <= -bound_y)
		{
			p.velocity.y *= -e;
			p.position.y = -bound_y + p.radius;
		}
		else if (p.position.y + p.radius >= bound_y)
		{
			p.velocity.y *= -e;
			p.position.y = bound_y - p.radius;
		}

		if (p.position.x - p.radius <= -bound_x)
		{
			p.velocity.x *= -e;
			p.position.x = -bound_x + p.radius;
		}
		else if (p.position.x + p.radius >= bound_x)
		{
			p.velocity.x *= -e;
			p.position.x = bound_x - p.radius;
		}
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
			float j = -(1 + e) * velocity_along_normal / (1 / m1 + 1 / m2);
			glm::vec2 impulse = j * normal;

			p1.velocity += impulse / m1;
			p2.velocity -= impulse / m2;

			//resolving interpenetration
			//the weights ensure that the heavier mass moves less
			float interpenetration = p1.radius + p2.radius - distance;
			if (interpenetration > 0.0f)
			{
				glm::vec2 offset = (interpenetration / (m1 + m2)) * normal;
				p1.position += offset * m2;
				p2.position -= offset * m1;
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
		float G_mod = 6.6743 * pow(10, -1) * p1.pixelsPerMeter;

		p1.acceleration -= G_mod * (m2 / r2) * dir;
		p2.acceleration += G_mod * (m1 / r2) * dir;
	}
};

#endif
