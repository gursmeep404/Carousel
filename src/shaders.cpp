#include "shaders.h"
#include <iostream>
#include <cmath>

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";


const char* fragmentShaderSource = R"(
#version 330 core
out vec4 color;

uniform float time;  // Time to animate the rainbow effect

void main() {
    float speed = 0.2; // Adjust this to control how fast the color cycles
    float rainbow = sin(time * speed) * 0.5 + 0.5;

    float r = 0.5 + 0.5 * sin(6.2831 * rainbow);
    float g = 0.5 + 0.5 * sin(6.2831 * rainbow + 2.0);
    float b = 0.5 + 0.5 * sin(6.2831 * rainbow + 4.0);

    vec3 baseColor = vec3(r, g, b);
    baseColor = mix(baseColor, vec3(1.0), 0.3);  // Lighten to neon pastel

    color = vec4(baseColor, 1.0);
}
)";




GLuint shaderProgram;

// Function to create shaders
void createShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader compilation failed:\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}


