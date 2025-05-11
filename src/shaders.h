#ifndef SHADERS_H
#define SHADERS_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern const char* vertexShaderSource;
extern const char* fragmentShaderSource;
extern GLuint shaderProgram;

void createShaders();

#endif

