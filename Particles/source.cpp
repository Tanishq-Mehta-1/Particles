#include <shader.h>
#include <vector>
#include <glfw/glfw3.h>


int setup(int width, int height, GLFWwindow* &window);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void drawCircle(unsigned int VAO);
static void circleSetup(glm::vec2 center, int res, unsigned int& VAO, unsigned int& VBO);
void Render(unsigned int VAO);

int width = 800;
int height = 800;
GLFWwindow* window{};

glm::vec2 center{0.0f};
int res = 30;
const float PI = 3.14;


int main()
{
	if (setup(width, height, window))
		std::cout << "ERROR::SETUP\n";
	//setting up shaders
	Shader objectShader{ "vertexShader.glsl", "fragmentShader.glsl" };
	unsigned int VAO, VBO;
	circleSetup(center, res, VAO, VBO);

	while (!glfwWindowShouldClose(window))
	{
		//inputs
		processInput(window);
		
		objectShader.use();
		Render(VAO);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

int setup(int width, int height, GLFWwindow* &window) { 

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

void Render(unsigned int VAO)
{
	//background colour
	glm::vec3 bgCol = glm::vec3(0.5f);
	glClearColor(bgCol.x, bgCol.y, bgCol.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//draw circle
	drawCircle(VAO);
}

static void drawCircle( unsigned int VAO)
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, res + 2);
	glBindVertexArray(0);
}

static void circleSetup(glm::vec2 center, int res, unsigned int& VAO, unsigned int& VBO)
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