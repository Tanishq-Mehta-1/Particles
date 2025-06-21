#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include <shader.h>
#include <random>
#include <vector>
#include <array>
#include <glfw/glfw3.h>
#include "particle.h"

//for fluid like behaviour, increase the num of particles

#define PI 3.14

int setup(int width, int height, GLFWwindow*& window);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void circleGenerate(glm::vec2 center, int res, unsigned int& VAO, unsigned int& VBO);
void ImGui_Setup(GLFWwindow* window);
void handleParticleNum(int& prevNum, int& particleNum, std::vector<Particle>& points, std::array<int, 2>& sizes, std::array<int, 2>& prevSize);
void spawnParticles(int no_of_particles, std::vector<Particle>& points, int size_min, int size_max);
float getRandom(float min, float max);

int width = 1920;
int height = 1080;
float deltaTime{ 0.0f };
float currentTime = { 0.0f };
float lastTime = { 0.0f };
std::mt19937 rng(currentTime);
GLFWwindow* window{};

int res = 20;
unsigned int circleVAO, circleVBO; //vertex and array buffers

int main()
{
	int particleNum = 300;

	if (setup(width, height, window))
		std::cout << "ERROR::SETUP\n";
	ImGuiIO& io = ImGui::GetIO(); //getting the io object

	std::vector<Particle> points;
	spawnParticles(particleNum, points,5,7);

	//setting up shaders
	Shader objectShader{ "vertexShader.vert", "fragmentShader.frag" };

	//configurable parameters and windows
	glm::vec3 particleColor;
	float acc_x{ 0.0f };
	float acc_y{ 0.0f };
	float e{ 0.8f };
	float waveStrength{ 10.0f };
	int prevNum{ particleNum };
	bool wave_motion{ false };
	bool chaos{ false };
	bool circle{ false };
	bool astronomical{ false };
	bool mirrorX{ false };
	bool velocity_colour{ true };
	bool collision_colour{ false };
	std::array<int, 2> particleSizes{ 5,7 };
	std::array<int, 2> prevSizes{ 5,7 };


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
			const char* items[] = { "None", "Wave", "Chaos", "Circle", "Astronomical" };
			bool* options[] = { &wave_motion, &chaos, &circle, &astronomical };

			ImGui::Begin("Sidebar", nullptr,
				ImGuiWindowFlags_NoMove
			);
			ImGui::Checkbox("Enable Velocity-based Colouring", &velocity_colour);
			ImGui::Checkbox("Enable Collision-based Colouring", &collision_colour);

			if (!velocity_colour && !collision_colour)
				ImGui::ColorPicker3("clear color", (float*)&particleColor);

			ImGui::Text("\nParticles: ");
			if (!astronomical)
			{
				ImGui::SliderFloat("X Acceleration", &acc_x, -10.0f, 10.0f);
				ImGui::SliderFloat("Y Acceleration", &acc_y, -10.0f, 10.0f);
			}
			float particles_max = astronomical ? 2000 : 5000;
			if (astronomical && particleNum > 2000)
				particleNum = 2000;

			ImGui::SliderInt("Number of Particle", &particleNum, 0, particles_max);
			ImGui::SliderFloat("Restitution Coefficient", &e, 0.0f, 1.0f);
			ImGui::InputInt("Minimum Size", &particleSizes[0]);
			ImGui::InputInt("Maximum Size", &particleSizes[1]);
			
			//size check
			if (particleSizes[0] > particleSizes[1])
			particleSizes[0] = particleSizes[1];

			particleSizes[0] = glm::clamp(particleSizes[0], 0, 30);
			particleSizes[1] = glm::clamp(particleSizes[1], 0, 30);

			//dropdown menu
			{
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
					*options[i - 1] = (item_current == i);
			}

			if (wave_motion)
				ImGui::SliderFloat("Wave Strength", &waveStrength, 1, 15);

			ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			//ImGui::Text("\nParticles generated: %d", points.size());
			sidebarWidth = ImGui::GetItemRectSize().x;
			ImGui::End();
		}

		//manually positioning the scene window
		{
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
		}

		//background colour
		glm::vec3 bgCol(0.0f);
		glClearColor(bgCol.x, bgCol.y, bgCol.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Render ImGui menus
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//scene render
		handleParticleNum(prevNum, particleNum, points, particleSizes, prevSizes);

		//handle collisions and gravity (for n^2 algo)
		for (int i = 0; i < particleNum; i++) {
			for (int j = i + 1; j < particleNum; j++)
			{
				if (astronomical)
					handleGravity(points[i], points[j]);

				handleParticleCollisions(points[i], points[j]);
			}
		}

		//pre-configured scenes
		if (wave_motion)
		{
			acc_x = sin(currentTime * 0.90) * waveStrength;
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

			if (!velocity_colour && !collision_colour) {
				points[i].colour = particleColor;
			}

			if (circle)
			{
				//for decent orbits, centrigufe must be greater than tangential (2,20) gives really good orbits
				float tangential_strength = 2.0f;
				float centrifugal_strength = 20.0f;

				//revolve around the viewport centre
				glm::vec2 posVec{ points[i].position }; //can really just use the position vector since the origin is the center

				//can obtain a tangent vector using a rotation matrix
				//for 2d space, the vector is essentially (-vec.y, vec.x)
				glm::vec2 tangent_acc{ -posVec.y, posVec.x };
				if (length(tangent_acc) > 0.0f)
					tangent_acc = glm::normalize(tangent_acc);

				//for circular motion, uniform, you really just need a centrifugal force i.e acceleration towards the centre
				glm::vec2 centrifugal_acc = glm::length(posVec) == 0.0f ? glm::vec2(0.0f) : -glm::normalize(posVec);

				acc_x = centrifugal_acc.x * centrifugal_strength + tangential_strength * tangent_acc.x;
				acc_y = centrifugal_acc.y * centrifugal_strength + tangential_strength * tangent_acc.y;
			}

			if (!astronomical)
				points[i].acceleration = points[i].pixelsPerMeter * glm::vec2(acc_x, acc_y);
			points[i].restitution_coefficient = e;
			points[i].update(deltaTime, window, velocity_colour, collision_colour);
			points[i].drawCircle(circleVAO, objectShader, res);
			if (astronomical)
				points[i].acceleration = glm::vec2(0.0f);
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

void handleParticleNum(int& prevNum, int& particleNum, std::vector<Particle>& points, std::array<int,2> &sizes, std::array<int, 2> &prevSize)
{
	if (prevSize[0] != sizes[0] || prevSize[1] != sizes[1])
	{
		//remove all points
		for (int i = 0; i < particleNum; i++)
			points.pop_back();
		//respawn all points
		spawnParticles(particleNum, points, sizes[0], sizes[1]);

		prevSize[0] = sizes[0]; prevSize[1] = sizes[1];
	}
	//handle spawns
	else if (prevNum != particleNum) {

		//spawn or despawn points
		if (prevNum > particleNum) //despawn
			for (int i = 0; i < prevNum - particleNum; i++)
				points.pop_back();
		else //spawn
			spawnParticles(particleNum - prevNum, points, sizes[0], sizes[1]);
	}
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

void spawnParticles(int no_of_particles, std::vector<Particle>& points, int size_min, int size_max)
{
	for (int i = 0; i < no_of_particles; i++)
	{
		//rand() - rand_max/2 to generate pos and negative numbers in the range [-rand_max/2 , rand_max/2]
		float pos_y = getRandom(0, height) - height / 2.0f;
		float pos_x = getRandom(0, width) - width / 2.0f;

		float r = getRandom(size_min, size_max);

		float max{ 10000 };
		float R = getRandom(0, max) / max;
		float G = getRandom(0, max) / max;
		float B = getRandom(0, max) / max;
		float alpha = getRandom(0.2 * max, max) / max;

		Particle particle(r, glm::vec2(pos_x, pos_y), window, glm::vec3(R, G, B), 1.0f, alpha);
		points.push_back(particle);
	}
}

float getRandom(float min, float max) {
	int offset = rng() % (int)(max - min + 1);

	if (min > max)
		return max;
	else if (min == max)
		return min;

	return min + offset;
}