cmake_minimum_required(VERSION 3.10)
project(Rooh)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Include directories for external libraries
include_directories(
    external/glad
    external/glfw/include
    external/kissfft
    external/stb
)

# Add the source files for each external library
set(EXTERNAL_LIBS
    external/glad/glad.c
    external/kissfft/kiss_fft.c
    external/stb/stb_vorbis.c
)

# Link libraries (link against OpenGL and GLFW)
target_link_libraries(your_target_name PRIVATE glfw OpenGL)


# Add executable
add_executable(Rooh src/main.cpp external/glad/src/glad.c external/kissfft/kiss_fft.c)

# Link libraries to the executable
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(Rooh OpenGL::GL glfw dl)
