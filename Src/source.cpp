#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include "Headers/gridLookup.h"
#include "Headers/helpers.h"
#include "Headers/particle.h"
#include "Headers/setup.h"
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <shader.h>
#include <vector>

#define PI 3.14

void handleParticleNum(int &prevNum, int &particleNum,
                       std::vector<Particle> &points, std::array<int, 2> &sizes,
                       std::array<int, 2> &prevSize, GridLookup &grid,
                       glm::vec2 windowSize);
void spawnParticles(int no_of_particles, std::vector<Particle> &points,
                    int size_min, int size_max);

// Global Variables
int width = 1280;
int height = 720;
float deltaTime{0.0f};
float currentTime = {0.0f};
float lastTime = {0.0f};
int maxParticles{30000};
GLFWwindow *window{};

int res = 40;
unsigned int circleVAO, circleVBO;
int particleNum = 300;
constexpr int SIDEBAR_WIDTH = 320;

std::vector<Particle> points;
glm::vec2 windowSize;
GridLookup *grid = nullptr;
Shader *objectShader = nullptr;

glm::vec3 particleColor;
float acc_x{0.0f};
float acc_y{0.0f};
float e{0.8f};
float waveStrength{10.0f};
int prevNum{particleNum};
bool wave_motion{false};
bool chaos{false};
bool circle{false};
bool astronomical{false};
bool mirrorX{false};
bool velocity_colour{true};
bool collision_colour{false};
std::array<int, 2> particleSizes{5, 7};
std::array<int, 2> prevSizes{5, 7};

unsigned int Model_Projections;
unsigned int Colors;
std::vector<glm::mat4> model_projections;
std::vector<glm::vec3> colors;

void main_loop() {
  ImGuiIO &io = ImGui::GetIO();

  lastTime = currentTime;
  currentTime = glfwGetTime();
  deltaTime = currentTime - lastTime;

  prevNum = particleNum;
  processInput(window);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  const ImVec2 displaySize = io.DisplaySize;
  const int sceneWidth =
      glm::max(1, static_cast<int>(displaySize.x - SIDEBAR_WIDTH));
  const int sceneHeight = glm::max(1, static_cast<int>(displaySize.y));

  {
    static int item_current = 0;
    const char *items[] = {"None", "Wave", "Chaos", "Circle", "Astronomical"};
    bool *options[] = {&wave_motion, &chaos, &circle, &astronomical};

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(static_cast<float>(SIDEBAR_WIDTH), displaySize.y),
        ImGuiCond_Always);

    ImGui::Begin("Sidebar", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse);

    ImGui::Checkbox("Enable Velocity-based Colouring", &velocity_colour);
    ImGui::Checkbox("Enable Collision-based Colouring", &collision_colour);

    if (!velocity_colour && !collision_colour)
      ImGui::ColorPicker3("clear color", (float *)&particleColor);

    ImGui::Text("\nParticles:");
    if (!astronomical) {
      ImGui::SliderFloat("X Acceleration", &acc_x, -10.0f, 10.0f);
      ImGui::SliderFloat("Y Acceleration", &acc_y, -10.0f, 10.0f);
    }

    int particles_max = astronomical ? 2000 : maxParticles;
    if (astronomical && particleNum > 2000)
      particleNum = 2000;

    ImGui::SliderInt("Number of Particle", &particleNum, 0, particles_max);
    ImGui::SliderFloat("Restitution Coefficient", &e, 0.0f, 1.0f);
    ImGui::InputInt("Minimum Size", &particleSizes[0]);
    ImGui::InputInt("Maximum Size", &particleSizes[1]);

    if (particleSizes[0] > particleSizes[1])
      particleSizes[0] = particleSizes[1];

    particleSizes[0] = glm::clamp(particleSizes[0], 1, 50);
    particleSizes[1] = glm::clamp(particleSizes[1], 1, 50);

    ImGui::Text("Some pre-configured scenes:");
    if (ImGui::BeginCombo("Pre-configured Scenes", items[item_current])) {
      for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
        bool is_selected = (item_current == i);
        if (ImGui::Selectable(items[i], is_selected))
          item_current = i;
        if (is_selected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    for (int i = 1; i < IM_ARRAYSIZE(items); i++)
      *options[i - 1] = (item_current == i);

    if (wave_motion)
      ImGui::SliderFloat("Wave Strength", &waveStrength, 1.0f, 15.0f);

    ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);

    ImGui::End();
  }

  {
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(SIDEBAR_WIDTH), 0),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(static_cast<float>(sceneWidth), static_cast<float>(sceneHeight)),
        ImGuiCond_Always);

    ImGui::Begin("Scene", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::End();
  }

  int framebufferWidth = 0;
  int framebufferHeight = 0;
  glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

  const float scaleX =
      displaySize.x > 0.0f
          ? static_cast<float>(framebufferWidth) / displaySize.x
          : 1.0f;

  const int sidebarPixels = static_cast<int>(SIDEBAR_WIDTH * scaleX);
  const int scenePixelsWidth = glm::max(0, framebufferWidth - sidebarPixels);

  glViewport(sidebarPixels, 0, scenePixelsWidth, framebufferHeight);
  glScissor(sidebarPixels, 0, scenePixelsWidth, framebufferHeight);
  glEnable(GL_SCISSOR_TEST);

  glm::vec3 bgCol(0.0f);
  glClearColor(bgCol.x, bgCol.y, bgCol.z, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glm::vec2 currentWindowSize(sceneWidth, sceneHeight);
  handleParticleNum(prevNum, particleNum, points, particleSizes, prevSizes,
                    *grid, currentWindowSize);

  grid->buildGrid(points, particleSizes[1], currentWindowSize);
  if (astronomical)
    grid->resolveGravity(points);
  grid->resolveCollisions(points);

  if (wave_motion) {
    acc_x = sin(currentTime * 0.90) * waveStrength;
    acc_y = -9.8f;
    e = 0.636;
  }

  if (chaos) {
    acc_x = getRandom(0, 10.0f) - 5.0f;
    acc_y = getRandom(0, 10.0f) - 5.0f;
    e = 1.0f;
  }

  glm::mat4 projection =
      glm::ortho(-sceneWidth / 2.0f, sceneWidth / 2.0f, -sceneHeight / 2.0f,
                 sceneHeight / 2.0f, -1.0f, 1.0f);

  for (Particle &p : points)
    p.setSimulationSize(sceneWidth, sceneHeight);

  objectShader->use();
  objectShader->setMat4("projection", projection);

  for (int i = 0; i < particleNum; i++) {
    if (i % 2 == 0 && mirrorX)
      acc_y *= -1;

    if (!velocity_colour && !collision_colour) {
      points[i].colour = particleColor;
    }

    if (circle) {
      float tangential_strength = 2.0f;
      float centrifugal_strength = 20.0f;

      glm::vec2 posVec{points[i].position};

      glm::vec2 tangent_acc{-posVec.y, posVec.x};
      if (length(tangent_acc) > 0.0f)
        tangent_acc = glm::normalize(tangent_acc);

      glm::vec2 centrifugal_acc = glm::length(posVec) == 0.0f
                                      ? glm::vec2(0.0f)
                                      : -glm::normalize(posVec);

      acc_x = centrifugal_acc.x * centrifugal_strength +
              tangential_strength * tangent_acc.x;
      acc_y = centrifugal_acc.y * centrifugal_strength +
              tangential_strength * tangent_acc.y;
    }

    if (!astronomical)
      points[i].acceleration =
          points[i].pixelsPerMeter * glm::vec2(acc_x, acc_y);
    points[i].restitution_coefficient = e;
    points[i].update(deltaTime, window, velocity_colour, collision_colour);

    if (astronomical)
      points[i].acceleration = glm::vec2(0.0f);
  }

  model_projections.clear();
  colors.clear();

  for (Particle &p : points) {
    model_projections.push_back(p.getModel());
    colors.push_back(p.colour);
  }

  objectShader->use();
  glBindVertexArray(circleVAO);

  glBindBuffer(GL_ARRAY_BUFFER, Model_Projections);
  glBufferSubData(GL_ARRAY_BUFFER, 0, particleNum * sizeof(glm::mat4),
                  model_projections.data());

  glBindBuffer(GL_ARRAY_BUFFER, Colors);
  glBufferSubData(GL_ARRAY_BUFFER, 0, particleNum * sizeof(glm::vec3),
                  colors.data());

  glDrawArraysInstanced(GL_TRIANGLES, 0, res * 3, particleNum);
  glBindVertexArray(0);

  glDisable(GL_SCISSOR_TEST);

  glfwPollEvents();
  glfwSwapBuffers(window);
}

int main() {
  if (setup(width, height, window, res, circleVAO, circleVBO))
    std::cout << "ERROR::SETUP\n";

  glfwGetWindowSize(window, &width, &height);

  width = glm::max(1, width - SIDEBAR_WIDTH);
  spawnParticles(particleNum, points, 5, 7);

  // Initialize objects now that OpenGL and GLFW are setup
  windowSize = points[0].getWindowSize();
  grid = new GridLookup(windowSize[0], windowSize[1], 7);

#ifdef __EMSCRIPTEN__
  objectShader = new Shader("Src/Shaders/vertexShader_web.vert",
                            "Src/Shaders/fragmentShader_web.frag");
#else
  objectShader = new Shader("Src/Shaders/vertexShader.vert",
                            "Src/Shaders/fragmentShader.frag");
#endif

  instancedArraySetup(Model_Projections, Colors, maxParticles, circleVAO);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 1);
#else
  while (!glfwWindowShouldClose(window)) {
    main_loop();
  }
#endif
  // Cleanup dynamically allocated resources
  delete grid;
  delete objectShader;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void handleParticleNum(int &prevNum, int &particleNum,
                       std::vector<Particle> &points, std::array<int, 2> &sizes,
                       std::array<int, 2> &prevSize, GridLookup &grid,
                       glm::vec2 windowSize) {
  if (prevSize[0] != sizes[0] || prevSize[1] != sizes[1]) {

    grid.updateGrid(sizes[1], windowSize);

    points.clear();
    spawnParticles(particleNum, points, sizes[0], sizes[1]);

    prevSize[0] = sizes[0];
    prevSize[1] = sizes[1];
  } else if (prevNum != particleNum) {
    if (prevNum > particleNum)
      for (int i = 0; i < prevNum - particleNum; i++)
        points.pop_back();
    else
      spawnParticles(particleNum - prevNum, points, sizes[0], sizes[1]);
  }
}

void spawnParticles(int no_of_particles, std::vector<Particle> &points,
                    int size_min, int size_max) {
  for (int i = 0; i < no_of_particles; i++) {
    float pos_y = getRandom(0, height) - height / 2.0f;
    float pos_x = getRandom(0, width) - width / 2.0f;

    float r = getRandom(size_min, size_max);

    float max{10000};
    float R = getRandom(0, max) / max;
    float G = getRandom(0, max) / max;
    float B = getRandom(0, max) / max;
    float alpha = getRandom(0.2 * max, max) / max;

    Particle particle(r, glm::vec2(pos_x, pos_y), window, glm::vec3(R, G, B),
                      1.0f, alpha);
    points.push_back(particle);
  }
}