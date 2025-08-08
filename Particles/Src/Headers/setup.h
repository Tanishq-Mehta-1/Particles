#ifndef SETUP_H
#define SETUP_H

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <random>
#include "Headers/particle.h"

constexpr float PI = 3.14;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void ImGui_Setup(GLFWwindow* window);
int setup(int width, int height, GLFWwindow*& window, int res, unsigned int& circleVAO, unsigned int& circleVBO);
static void circleGenerate(glm::vec2 center, int res, unsigned int& VAO, unsigned int& VBO);

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void ImGui_Setup(GLFWwindow* window)
{
	//setting up imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags != ImGuiConfigFlags_NavEnableKeyboard; //enable keyboard controls
	io.ConfigFlags != ImGuiConfigFlags_NavEnableGamepad; //enable gamepad controls

	ImGui::StyleColorsDark(); //sets the dark theme for windows
	ImGui_ImplGlfw_InitForOpenGL(window, true); //setup the platform backend
	ImGui_ImplOpenGL3_Init("#version 460");
}

int setup(int width, int height, GLFWwindow*& window, int res, unsigned int& circleVAO, unsigned int& circleVBO ) {

	//setting up glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Particle Simulator", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create window\n";
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	ImGui_Setup(window);

	//loading functions through GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD\n";
		return 1;
	}

	glViewport(0, 0, width, height);

	//enabling alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//generate the circle vertices
	//thus now, we just use these vertices to create a circle
	circleGenerate(glm::vec2(0.0f), res, circleVAO, circleVBO);

	return 0;
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

#endif // !SETUP_H

