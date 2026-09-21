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

// for fluid like behaviour, increase the num of particles
#define PI 3.14

void handleParticleNum(int &prevNum, int &particleNum,
                       std::vector<Particle> &points, std::array<int, 2> &sizes,
                       std::array<int, 2> &prevSize, GridLookup &grid,
                       glm::vec2 windowSize);
void spawnParticles(int no_of_particles, std::vector<Particle> &points,
                    int size_min, int size_max);

int width = 1280;
int height = 720;
float deltaTime{0.0f};
float currentTime = {0.0f};
float lastTime = {0.0f};
int maxParticles{30000};
GLFWwindow *window{};

int res = 40;
unsigned int circleVAO, circleVBO; // vertex and array buffers

int main() {
  int particleNum = 300;

  if (setup(width, height, window, res, circleVAO, circleVBO))
    std::cout << "ERROR::SETUP\n";
  ImGuiIO &io = ImGui::GetIO(); // getting the io object

  // setup() can clamp the requested size to the monitor work area.
  glfwGetWindowSize(window, &width, &height);

  constexpr int SIDEBAR_WIDTH = 320;

  std::vector<Particle> points;
  width = glm::max(1, width - SIDEBAR_WIDTH);
  spawnParticles(particleNum, points, 5, 7);

  glm::vec2 windowSize = points[0].getWindowSize();
  GridLookup grid(windowSize[0], windowSize[1], 7);

  // setting up shaders
  Shader objectShader{"Src/Shaders/vertexShader.vert",
                      "Src/Shaders/fragmentShader.frag"};

  // configurable parameters and windows
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

  instancedArraySetup(Model_Projections, Colors, maxParticles, circleVAO);

  std::vector<glm::mat4> model_projections;
  std::vector<glm::vec3> colors;

  while (!glfwWindowShouldClose(window)) {
    lastTime = currentTime;
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;

    prevNum = particleNum;
    // inputs
    processInput(window);

    // starting the Dear Imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ------------------------------------------------------------
    // UI layout
    // ------------------------------------------------------------
    // ImGui uses logical window pixels. Physics uses the scene
    // dimensions, which exclude the sidebar.
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
      ImGui::SetNextWindowSize(ImVec2(static_cast<float>(sceneWidth),
                                      static_cast<float>(sceneHeight)),
                               ImGuiCond_Always);

      ImGui::Begin("Scene", nullptr,
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

      ImGui::End();
    }

    // Convert the logical sidebar width to physical framebuffer pixels.
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

    // background colour
    glm::vec3 bgCol(0.0f);
    glClearColor(bgCol.x, bgCol.y, bgCol.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render ImGui menus
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // scene render
    glm::vec2 currentWindowSize(sceneWidth, sceneHeight);
    handleParticleNum(prevNum, particleNum, points, particleSizes, prevSizes,
                      grid, currentWindowSize);

    grid.buildGrid(points, particleSizes[1], currentWindowSize);
    if (astronomical)
      grid.resolveGravity(points);
    grid.resolveCollisions(points);

    // pre-configured scenes
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

    // update and then draw
    //  Projection is expressed in logical scene coordinates.
    //  The OpenGL viewport above maps it to the physical framebuffer.
    glm::mat4 projection =
        glm::ortho(-sceneWidth / 2.0f, sceneWidth / 2.0f, -sceneHeight / 2.0f,
                   sceneHeight / 2.0f, -1.0f, 1.0f);

    // Physics bounds must match the scene, not the full GLFW window.
    for (Particle &p : points)
      p.setSimulationSize(sceneWidth, sceneHeight);

    objectShader.use();
    objectShader.setMat4("projection", projection);

    for (int i = 0; i < particleNum; i++) {
      if (i % 2 == 0 && mirrorX)
        acc_y *= -1;

      if (!velocity_colour && !collision_colour) {
        points[i].colour = particleColor;
      }

      if (circle) {
        // for decent orbits, centrigufe must be greater than tangential (2,20)
        // gives really good orbits
        float tangential_strength = 2.0f;
        float centrifugal_strength = 20.0f;

        // revolve around the viewport centre
        glm::vec2 posVec{
            points[i].position}; // can really just use the position vector
                                 // since the origin is the center

        // can obtain a tangent vector using a rotation matrix
        // for 2d space, the vector is essentially (-vec.y, vec.x)
        glm::vec2 tangent_acc{-posVec.y, posVec.x};
        if (length(tangent_acc) > 0.0f)
          tangent_acc = glm::normalize(tangent_acc);

        // for circular motion, uniform, you really just need a centrifugal
        // force i.e acceleration towards the centre
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
      // extremely slow process, since we are sending gpu information for every
      // particle, can be made better using instancing
    }

    model_projections.clear();
    colors.clear();

    for (Particle &p : points) {
      model_projections.push_back(p.getModel());
      colors.push_back(p.colour);
    }

    // setup instanced arrays
    objectShader.use();
    glBindVertexArray(circleVAO);

    // setup MVP
    glBindBuffer(GL_ARRAY_BUFFER, Model_Projections);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particleNum * sizeof(glm::mat4),
                    model_projections.data());

    // setup Colours
    glBindBuffer(GL_ARRAY_BUFFER, Colors);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particleNum * sizeof(glm::vec3),
                    colors.data());

    // draw call
    glDrawArraysInstanced(GL_TRIANGLES, 0, res * 3, particleNum);
    glBindVertexArray(0);

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

void handleParticleNum(int &prevNum, int &particleNum,
                       std::vector<Particle> &points, std::array<int, 2> &sizes,
                       std::array<int, 2> &prevSize, GridLookup &grid,
                       glm::vec2 windowSize) {
  if (prevSize[0] != sizes[0] || prevSize[1] != sizes[1]) {

    grid.updateGrid(sizes[1], windowSize);

    // Safely clear the vector instead of popping individual elements
    points.clear();

    // respawn all points
    spawnParticles(particleNum, points, sizes[0], sizes[1]);

    prevSize[0] = sizes[0];
    prevSize[1] = sizes[1];
  }
  // handle spawns
  else if (prevNum != particleNum) {

    // spawn or despawn points
    if (prevNum > particleNum) // despawn
      for (int i = 0; i < prevNum - particleNum; i++)
        points.pop_back();
    else // spawn
      spawnParticles(particleNum - prevNum, points, sizes[0], sizes[1]);
  }
}

void spawnParticles(int no_of_particles, std::vector<Particle> &points,
                    int size_min, int size_max) {
  for (int i = 0; i < no_of_particles; i++) {
    // rand() - rand_max/2 to generate pos and negative numbers in the range
    // [-rand_max/2 , rand_max/2]
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