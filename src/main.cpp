#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>
#include <stb_vorbis.c>
#include <kiss_fft.h>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
void main() {
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
})";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 color;
void main() {
    color = vec4(0.0, 1.0, 0.0, 1.0); // Green color for demo
})";

GLFWwindow* window;
GLuint shaderProgram;
GLuint VAO, VBO;

void initOpenGL();
void createShaders();
void setupBuffers();
void loadAudio(const char* filename);
void processAudio();
void renderScene();
void cleanup();

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    window = glfwCreateWindow(800, 600, "Audio Visualizer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize OpenGL
    initOpenGL();
    createShaders();
    setupBuffers();
    loadAudio("assets/song.ogg");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processAudio();  // Process audio (FFT)
        renderScene();   // Render visual effects

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}

void initOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader\n";
        exit(EXIT_FAILURE);
    }
}

void createShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void setupBuffers() {
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f, 
         0.5f, -0.5f, 0.0f, 
         0.0f,  0.5f, 0.0f  
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void loadAudio(const char* filename) {
    // Load audio using stb_vorbis and kissFFT here
    // Decode the audio file and prepare it for FFT processing
    // Process audio data and store frequencies to update visuals
}

void processAudio() {
    // Perform FFT on the audio to get frequency data
    // For each frame of audio, update your visual effects
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);  // Example: draw triangle
    glBindVertexArray(0);
}

void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
