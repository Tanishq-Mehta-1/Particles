cmake_minimum_required(VERSION 3.10)
project(ParticleSimulator)
set(CMAKE_CXX_STANDARD 17)

# Include current directory so <shader.h> and <ImGui/imgui.h> are found
include_directories(.)

# Compile source.cpp alongside necessary library implementations
add_executable(particles
    Src/source.cpp
    glad.c
    ImGui/imgui.cpp
    ImGui/imgui_draw.cpp
    ImGui/imgui_tables.cpp
    ImGui/imgui_widgets.cpp
    ImGui/imgui_impl_glfw.cpp
    ImGui/imgui_impl_opengl3.cpp
)

target_include_directories(particles PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Src)

# Find system libraries installed via pacman
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# Link GLFW and OpenGL to the executable
target_link_libraries(particles glfw OpenGL::GL)