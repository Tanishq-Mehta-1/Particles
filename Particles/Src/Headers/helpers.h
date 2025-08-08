#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <random>
#include <glm/glm.hpp>

//utility functions for the programs
std::mt19937 rng(0.0f);
float getRandom(float min, float max) {
	int offset = rng() % (int)(max - min + 1);

	if (min > max)
		return max;
	else if (min == max)
		return min;

	return min + offset;
}

void displayVec2(glm::vec2 vector)
{
	std::cout << vector.x << ' ' << vector.y << '\n';
}

void displayVec3(glm::vec3 vector)
{
	std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << '\n';
}

#endif