#include "renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <kiss_fft.h>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include "audio.h"
#include "cleanup.h"

extern GLuint shaderProgram;
extern GLFWwindow* window;

const int fftSize = 512;
const int numBars = fftSize / 2;
GLuint barVAO[numBars], barVBO[numBars];
float barHeights[numBars] = {0};

kiss_fft_cfg fftCfg;
kiss_fft_cpx* fftInput;
kiss_fft_cpx* fftOutput;

void initOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        exit(-1);
    }
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);
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
        barHeights[i] = std::min(magnitude / 5000.0f, 3.0f); // Cap height to prevent overflow
    }

    playbackIndex += fftSize / 2;
}

void renderScene() {
    glClearColor(0.0f, 0.0f, 0.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // View and Projection matrices
    float time = glfwGetTime();
    glm::vec3 camPos = glm::vec3(10.0f * sin(time * 0.5f), 5.0f, 10.0f * cos(time * 0.5f));
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    for (int i = 0; i < numBars; ++i) {
        float angle = (2.0f * M_PI / numBars) * i;
        float radius = 5.0f;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float h = barHeights[i];

        float w = 0.1f; // width of bar
        float d = 0.05f; // depth

        float vertices[] = {
    // positions (12 triangles * 3 vertices = 36 vertices)

    // Front face
    -w, 0.0f,  d,   w, 0.0f,  d,   w, h, d,
    -w, 0.0f,  d,   w, h,  d,  -w, h, d,

    // Back face
    -w, 0.0f, -d,  -w, h, -d,   w, h, -d,
    -w, 0.0f, -d,   w, h, -d,   w, 0.0f, -d,

    // Left face
    -w, 0.0f,  d,  -w, h,  d,  -w, h, -d,
    -w, 0.0f,  d,  -w, h, -d,  -w, 0.0f, -d,

    // Right face
     w, 0.0f,  d,   w, 0.0f, -d,   w, h, -d,
     w, 0.0f,  d,   w, h, -d,   w, h,  d,

    // Top face
    -w, h,  d,   w, h,  d,   w, h, -d,
    -w, h,  d,   w, h, -d,  -w, h, -d,

    // Bottom face
    -w, 0.0f,  d,  -w, 0.0f, -d,   w, 0.0f, -d,
    -w, 0.0f,  d,   w, 0.0f, -d,   w, 0.0f,  d
};


        GLuint indices[] = {
            0, 1, 2, 2, 3, 0, // Front
            4, 5, 6, 6, 7, 4, // Back
            0, 3, 5, 5, 4, 0, // Left
            1, 7, 6, 6, 2, 1, // Right
            3, 2, 6, 6, 5, 3, // Top
            0, 4, 7, 7, 1, 0  // Bottom
        };

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, 0.0f, z));
        model = glm::rotate(model, -angle, glm::vec3(0, 1, 0));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(barVAO[i]);
glBindBuffer(GL_ARRAY_BUFFER, barVBO[i]);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

glDrawArrays(GL_TRIANGLES, 0, 36);

    }
}

void cleanup() {
    if (fftCfg) {
        free(fftCfg);
        fftCfg = nullptr;
    }
    if (fftInput) {
        free(fftInput);
        fftInput = nullptr;
    }
    if (fftOutput) {
        free(fftOutput);
        fftOutput = nullptr;
    }

    for (int i = 0; i < numBars; ++i) {
        if (barVAO[i]) {
            glDeleteVertexArrays(1, &barVAO[i]);
            barVAO[i] = 0;
        }
        if (barVBO[i]) {
            glDeleteBuffers(1, &barVBO[i]);
            barVBO[i] = 0;
        }
    }

    if (audioData) {
        free(audioData);
        audioData = nullptr;
    }

    if (source) {
        alDeleteSources(1, &source);
        source = 0;
    }

    if (buffer) {
        alDeleteBuffers(1, &buffer);
        buffer = 0;
    }

    if (context) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        context = nullptr;
    }

    if (device) {
        alcCloseDevice(device);
        device = nullptr;
    }

}
