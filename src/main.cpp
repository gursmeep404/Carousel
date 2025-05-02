#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <kiss_fft.h>
#include <cmath>
#include <vector>

#define STB_VORBIS_IMPLEMENTATION
#include "../external/stb/stb_vorbis.c"


const int fftSize = 512;
const int numBars = fftSize / 2;

GLFWwindow* window;
GLuint shaderProgram;
GLuint barVAO[numBars], barVBO[numBars];
float barHeights[numBars] = {0};

short* audioData = nullptr;
int audioDataSize = 0;
int sampleRate = 0;
int playbackIndex = 0;

kiss_fft_cfg fftCfg;
kiss_fft_cpx* fftInput;
kiss_fft_cpx* fftOutput;

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
void main() {
    gl_Position = vec4(position, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 color;
void main() {
    color = vec4(0.0, 1.0, 0.5, 1.0); // Teal
}
)";

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

void initOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        exit(-1);
    }
    glViewport(0, 0, 800, 600);
}

void loadAudio(const char* filename) {
    int channels;
    audioDataSize = stb_vorbis_decode_filename(filename, &channels, &sampleRate, &audioData);
    if (!audioData) {
        std::cerr << "Audio load failed.\n";
        exit(-1);
    }
    std::cout << "Audio loaded: " << audioDataSize << " samples\n";
}

void setupBars() {
    for (int i = 0; i < numBars; ++i) {
        glGenVertexArrays(1, &barVAO[i]);
        glGenBuffers(1, &barVBO[i]);
    }
}

void processAudioFrame() {
    if (!fftCfg)
        fftCfg = kiss_fft_alloc(fftSize, 0, nullptr, nullptr);

    if (!fftInput)
        fftInput = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * fftSize);
    if (!fftOutput)
        fftOutput = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * fftSize);

    if (playbackIndex + fftSize >= audioDataSize)
        playbackIndex = 0;

    for (int i = 0; i < fftSize; ++i) {
        fftInput[i].r = audioData[playbackIndex + i];
        fftInput[i].i = 0.0f;
    }

    kiss_fft(fftCfg, fftInput, fftOutput);

    for (int i = 0; i < numBars; ++i) {
        float magnitude = sqrt(fftOutput[i].r * fftOutput[i].r + fftOutput[i].i * fftOutput[i].i);
        barHeights[i] = magnitude / 10000.0f;
    }

    playbackIndex += fftSize / 2;
}

void renderScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);

    for (int i = 0; i < numBars; ++i) {
        float x = -1.0f + 2.0f * i / numBars;
        float w = 1.8f / numBars;
        float h = barHeights[i];

        float vertices[] = {
            x, -1.0f, 0.0f,
            x, -1.0f + h, 0.0f,
            x + w, -1.0f + h, 0.0f,
            x + w, -1.0f, 0.0f
        };

        glBindVertexArray(barVAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, barVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

void cleanup() {
    if (audioData) free(audioData);
    if (fftCfg) free(fftCfg);
    if (fftInput) free(fftInput);
    if (fftOutput) free(fftOutput);

    for (int i = 0; i < numBars; ++i) {
        glDeleteVertexArrays(1, &barVAO[i]);
        glDeleteBuffers(1, &barVBO[i]);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    window = glfwCreateWindow(800, 600, "Audio Visualizer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    initOpenGL();
    createShaders();
    setupBars();
    loadAudio("../assets/willow.ogg");

    while (!glfwWindowShouldClose(window)) {
        processAudioFrame();
        renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}

