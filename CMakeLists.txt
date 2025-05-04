cmake_minimum_required(VERSION 3.10)
project(Rooh)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Include directories for external libraries
include_directories(
     ${CMAKE_SOURCE_DIR}/external/glad
      ${CMAKE_SOURCE_DIR}/glfw/include
      ${CMAKE_SOURCE_DIR}/external/kissfft
)

# Find required system packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenAL REQUIRED)

# Add executable
add_executable(Rooh 
    src/main.cpp 
    src/audio.cpp
    src/renderer.cpp
    src/shaders.cpp
    external/glad/glad.c 
    external/kissfft/kiss_fft.c 

)

# Link libraries to the executable
target_link_libraries(Rooh PRIVATE OpenGL::GL glfw OpenAL::OpenAL dl)

