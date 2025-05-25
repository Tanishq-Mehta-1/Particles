#include <shader.h>
#include <vector>
#include <glfw/glfw3.h>
#include "particle.h"
#include <chrono>
#include <thread>

#define PI 3.14

int setup(int width, int height, GLFWwindow* &window);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void circleGenerate(glm::vec2 center, int res, unsigned int& VAO, unsigned int& VBO);
float getRandom(float min, float max);

int width = 800;
int height = 800;
float deltaTime{ 0.0f };
float currentTime = { 0.0f };
float lastTime = { 0.0f };
GLFWwindow* window{};

int res = 100;

unsigned int circleVAO, circleVBO;


int main()
{
	if (setup(width, height, window))
		std::cout << "ERROR::SETUP\n";

	const int spawn_num = 50;
	std::vector<Particle> points;

	//randomly spawn points
	srand(glfwGetTime());
	for (int i = 0; i < spawn_num; i++)
	{
		/*float pos_y = getRandom( 0, 400);
		float pos_x = getRandom( 0, 400);*/

		float pos_x = i - 25.0f;
		float pos_y = 300.0f * sin(glm::radians(pos_x));
		float r = 5.0f;

		Particle particle(r, glm::vec2(pos_x ,pos_y), window);
		points.push_back(particle);
	}

	//setting up shaders
	Shader objectShader{ "vertexShader.glsl", "fragmentShader.glsl" };

	while (!glfwWindowShouldClose(window))
	{
		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;

		//inputs
		processInput(window);

		//background colour
		glm::vec3 bgCol = glm::vec3(0.5f);
		glClearColor(bgCol.x, bgCol.y, bgCol.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < spawn_num; i++)
		{
			points[i].drawCircle(circleVAO, objectShader, res);
			points[i].update(deltaTime, window);
			std::cout << "Position " << i << ' ' << points[i].position.x << ' ' << points[i].position.y << '\n';
		}

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void circleGenerate(glm::vec2 center, int res, unsigned int& VAO, unsigned int& VBO)
{
	//generating vertices
	std::vector<float> vertices;
	vertices.push_back(center.x);
	vertices.push_back(center.y);

	for (int i = 0; i <= res; i++) {
		glm::vec2 point;
		float theta = 2.0f * PI * float(i) / float(res);
		point.x = center.x + sin(theta) * 0.5;
		point.y = center.y + cos(theta) * 0.5;

		vertices.push_back(point.x);
		vertices.push_back(point.y);
	}
	//points lie between -0.5 and 0.5

	//draw the stuff
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

int setup(int width, int height, GLFWwindow*& window) {

	//setting up glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "First project!!!!", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create window\n";
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//loading functions through GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD\n";
		return 1;
	}

	glViewport(0, 0, width, height);

	//generate the circle vertices
	//thus now, we just use these vertices to create a circle
	circleGenerate(glm::vec2(0.0f), res, circleVAO, circleVBO);

	return 0;
}

float getRandom(float min, float max) {
	float num = rand();
	while (num <= min || num >= max){
		num = rand();
	}
	return num;
}