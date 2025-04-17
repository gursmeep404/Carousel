cmake_minimum_required(VERSION 3.10)
project(Rooh)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Include directories for libraries
include_directories(external/glad/include)
include_directories(external/glfw/include)
include_directories(external/stb)
include_directories(external/kissfft)

# Add executable
add_executable(Rooh src/main.cpp external/glad/src/glad.c external/kissfft/kiss_fft.c)

# Link libraries to the executable
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(Rooh OpenGL::GL glfw dl)
