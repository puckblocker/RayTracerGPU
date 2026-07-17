# Check Installed Version
cmake_minimum_required(VERSION 3.22.1)
# Project Name
project(gpu_ray_tracer VERSION 1.0.0)

# Find Package
find_package(OpenGL REQUIRED)

# Build
add_executable(gpu_ray_tracer
    src/main.cpp
    src/package_manager.cpp
    src/viewport.cpp 
    src/glad.c)

# Located Header Files
target_include_directories(gpu_ray_tracer PRIVATE dependencies)
# Link Libraries
target_link_libraries(gpu_ray_tracer ${CMAKE_CURRENT_SOURCE_DIR}/glfw3.lib)

# Copy Test & Compute Shader File to Debug
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scene.txt 
          ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer.comp
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)