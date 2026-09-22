cmake_minimum_required(VERSION 3.5)
project(ParticleSimulator)
set(CMAKE_CXX_STANDARD 17)

# Include current directory
include_directories(.)

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Compile executable
add_executable(particles
    Src/source.cpp
    ImGui/imgui.cpp
    ImGui/imgui_draw.cpp
    ImGui/imgui_tables.cpp
    ImGui/imgui_widgets.cpp
    ImGui/imgui_impl_glfw.cpp
    ImGui/imgui_impl_opengl3.cpp
)

if(NOT EMSCRIPTEN)
    target_sources(particles PRIVATE glad.c)
endif()

target_include_directories(particles PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Src)
# Link the fetched GLM library
target_link_libraries(particles PRIVATE glm)

if(EMSCRIPTEN)
    set_target_properties(particles PROPERTIES SUFFIX ".html")

    # Removed -sUSE_GLFW and -sUSE_WEBGL2 from compile options to fix the warnings
    target_link_options(particles PRIVATE
        -sUSE_GLFW=3
        -sUSE_WEBGL2=1
        -sWASM=1
        -sALLOW_MEMORY_GROWTH=1
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Src/Shaders@Src/Shaders
    )
else()
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(particles PRIVATE glfw OpenGL::GL)
endif()