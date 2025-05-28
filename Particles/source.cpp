#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include <shader.h>
#include <vector>
#include <glfw/glfw3.h>
#include "particle.h"

//for fluid like behaviour, increase the num of particles

#define PI 3.14

int setup(int width, int height, GLFWwindow*& window);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void circleGenerate(glm::vec2 center, int res, unsigned int& VAO, unsigned int& VBO);
void ImGui_Setup(GLFWwindow* window);
void handleParticleNum(int& prevNum, int& particleNum, std::vector<Particle>& points);
void spawnParticles(int no_of_particles, std::vector<Particle>& points);
float getRandom(float min, float max);

int width = 1440;
int height = 900;
float deltaTime{ 0.0f };
float currentTime = { 0.0f };
float lastTime = { 0.0f };
GLFWwindow* window{};

int res = 30;
unsigned int circleVAO, circleVBO; //vertex and array buffers

int main()
{
	int particleNum = 400; //works well till , with no overlap till ~300


	if (setup(width, height, window))
		std::cout << "ERROR::SETUP\n";
	ImGuiIO& io = ImGui::GetIO(); //getting the io object

	std::vector<Particle> points;
	spawnParticles(particleNum, points);

	//setting up shaders
	Shader objectShader{ "vertexShader.vert", "fragmentShader.frag" };

	//configurable parameters and windows
	glm::vec3 particleColor;
	float acc_x{ 0.0f };
	float acc_y{ -9.8f };
	float e{ 0.8f };
	int prevNum{ particleNum };
	bool wave_motion{ false };
	bool chaos{ false };
	bool mirrorX{ false };
	bool velocity_colour{ true };

	while (!glfwWindowShouldClose(window))
	{
		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;

		prevNum = particleNum;
		//inputs
		processInput(window);

		//starting the Dear Imgui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float sidebarWidth{};
		//Side bar
		{
			static int item_current = 0; //stores the index of selected item
			const char* items[] = {"None", "Wave", "Chaos"};
			bool* options[] = {&wave_motion, &chaos };

			ImGui::Begin("Sidebar", nullptr,
				ImGuiWindowFlags_NoMove 
			);
			ImGui::Checkbox("Enable Velocity-based Colouring", &velocity_colour);
			if(!velocity_colour)
				ImGui::ColorPicker3("clear color", (float*)&particleColor);

			ImGui::Text("\nParticles: ");
			ImGui::SliderFloat("X Acceleration", &acc_x, -10.0f, 10.0f);
			ImGui::SliderFloat("Y Acceleration", &acc_y, -10.0f, 10.0f);
			ImGui::SliderInt("Number of Particle", &particleNum, 0, 1000);
			ImGui::SliderFloat("Restitution Coefficient", &e, 0.0f, 1.0f);
			ImGui::Checkbox("Mirror horizontal", &mirrorX);

			//dropdown menu
			ImGui::Text("Some pre-configured scenes:");
			if (ImGui::BeginCombo("Pre-configured Scenes", items[item_current])) //second param shows the previewed item
			{
				for (int i = 0; i < IM_ARRAYSIZE(items); i++) {

					bool is_selected = (item_current == i);
					if (ImGui::Selectable(items[i], is_selected))
						item_current = i;

					if (is_selected) //what to do if selected
						ImGui::SetItemDefaultFocus(); //helper function that highlights this option the next time you open the dropdown menu
					
				}
				ImGui::EndCombo();
			}
		
			for (int i = 1; i < IM_ARRAYSIZE(items); i++)
				*options[i-1] = (item_current == i);

			ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			sidebarWidth = ImGui::GetItemRectSize().x;
			ImGui::End();
		}

		//manually positioning the scene window
		ImGui::SetNextWindowPos(ImVec2(sidebarWidth, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - sidebarWidth, io.DisplaySize.y));

		//scene window
		ImGui::Begin("Scene", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse
		);

		ImVec2 scenePos = ImGui::GetWindowPos();
		ImVec2 sceneSize = ImGui::GetWindowSize();
		ImGui::End();

		glViewport(sidebarWidth, io.DisplaySize.y - scenePos.y - sceneSize.y, sceneSize.x, sceneSize.y);
		glScissor(sidebarWidth, io.DisplaySize.y - scenePos.y - sceneSize.y, sceneSize.x, sceneSize.y);
		glEnable(GL_SCISSOR_TEST);

		//background colour
		glm::vec3 bgCol(0.0f);
		glClearColor(bgCol.x, bgCol.y, bgCol.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Render ImGui menus
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//scene render
		handleParticleNum(prevNum, particleNum, points);

		//handle collisions
		for (int i = 0; i < particleNum; i++) {
			for (int j = i + 1; j < particleNum; j++)
				handleParticleCollisions(points[i], points[j]);
		}

		//pre-configured scenes
		if (wave_motion)
		{
			acc_x = sin(currentTime * 0.90) * 10.0f;
			acc_y = -9.8f;
			e = 0.636;
		}

		if (chaos)
		{
			acc_x = getRandom(0, 10.0f) - 5.0f;
			acc_y = getRandom(0, 10.0f) - 5.0f;
			e = 1.0f;
		}

		//update and then draw
		for (int i = 0; i < particleNum; i++)
		{
			if (i % 2 == 0 && mirrorX)
				acc_y *= -1;

			if (!velocity_colour) {
				points[i].colour = particleColor;
			}

			points[i].acceleration = points[i].pixelsPerMeter * glm::vec2(acc_x, acc_y);
			points[i].restitution_coefficient = e;
			points[i].update(deltaTime, window, velocity_colour);
			points[i].drawCircle(circleVAO, objectShader, res);
			//extremely slow process, since we are sending gpu information for every particle, can be made better using instancing
		}

		glDisable(GL_SCISSOR_TEST);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
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

void handleParticleNum(int& prevNum, int& particleNum, std::vector<Particle>& points)
{
	//handle spawns
	if (prevNum != particleNum) {

		//spawn or despawn points
		if (prevNum > particleNum) //despawn
			for (int i = 0; i < prevNum - particleNum; i++)
				points.pop_back();
		else //spawn
			spawnParticles(particleNum - prevNum, points);
	}


	//handle barely visible particles
	auto it = points.begin();
	while (it != points.end())
	{

		if ((*it).alpha <= 0.1f)
			it = points.erase(it);
		else
			it++;
	}
	particleNum = points.size();
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

void spawnParticles(int no_of_particles, std::vector<Particle>& points)
{
	for (int i = 0; i < no_of_particles; i++)
	{
		//rand() - rand_max/2 to generate pos and negative numbers in the range [-rand_max/2 , rand_max/2]
		float pos_y = getRandom(0, 300);
		float pos_x = getRandom(0, width) - width / 2;
		float r = getRandom(7, 10); //works better with smaller radii

		/*float pos_y = i * 100.0f;
		float pos_x = 0;
		float r = 20.0f;*/

		float max{ 10000 };
		float R = getRandom(0, max) / max;
		float G = getRandom(0, max) / max;
		float B = getRandom(0, max) / max;
		float alpha = getRandom(0, max) / max;

		Particle particle(r, glm::vec2(pos_x, pos_y), window, glm::vec3(R, G, B), 1.0f, alpha);
		points.push_back(particle);
	}
}

float getRandom(float min, float max) {
	float num = rand();

	if (min > max)
		return max;

	if (min == max)
		return min;

	while (num <= min || num >= max) {
		num = rand();
	}
	return num;
}