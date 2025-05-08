#include "renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <kiss_fft.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include "audio.h"
#include "cleanup.h"
#include "particles.h"

extern GLuint shaderProgram;
extern GLFWwindow* window;

ParticleSystem particleSystem(1000);

const int fftSize = 512;
const int numBars = fftSize / 2;
GLuint barVAO[numBars], barVBO[numBars];
float barHeights[numBars] = {0};

float bassAmplitude = 1.0f;  // Base scale


GLuint baseCircleVAO, baseCircleVBO;
float baseCircleRadius = 5.0f;
const int circleSegments = 100;
float baseCircleVertices[(circleSegments + 2) * 3];  // +2 for center and first vertex of the circle

kiss_fft_cfg fftCfg;
kiss_fft_cpx* fftInput;
kiss_fft_cpx* fftOutput;

void initOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        exit(-1);
    }
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 1600, 900);
}

void setupBars() {
    for (int i = 0; i < numBars; ++i) {
        glGenVertexArrays(1, &barVAO[i]);
        glGenBuffers(1, &barVBO[i]);
    }
}

void setupBaseCircle() {
    float radius = 2.0f;
    int segments = 100;

    std::vector<float> vertices;

    // Center vertex (at origin in XZ plane)
    vertices.push_back(0.0f); // X
    vertices.push_back(0.0f); // Y (height)
    vertices.push_back(0.0f); // Z

    for (int i = 0; i <= segments; ++i) {
        float angle = (i / float(segments)) * 2.0f * M_PI;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = 0.0f; // Flat on X-Z plane
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    glGenVertexArrays(1, &baseCircleVAO);
    glGenBuffers(1, &baseCircleVBO);

    glBindVertexArray(baseCircleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, baseCircleVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



void renderBaseCircle() {
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));

    GLuint transformLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(rotation));

    glBindVertexArray(baseCircleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 1 + 100 + 1); // center + segments + repeat of first outer vertex
    glBindVertexArray(0);
}

void processAudioFrame() {
  static float threshold = 1.5f;
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
          // Emit particles if bar height exceeds threshold
        if (barHeights[i] > threshold) {
            glm::vec3 pos = glm::vec3(i * 1.5f, 0.0f, barHeights[i]);  // Position based on index and height
            glm::vec3 vel = glm::vec3(0.0f, 1.0f, 0.0f) * barHeights[i] * 0.5f;  // Example velocity
            particleSystem.emit(pos, vel);
        }
    }


    playbackIndex += fftSize / 2;
}



void renderScene() {
	glClearColor(0.0f, 0.0f, 0.0f, 0.05f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // View and Projection matrices
    float time = glfwGetTime();
    glm::vec3 camPos = glm::vec3(10.0f * sin(time * 0.5f), 5.0f, 10.0f * cos(time * 0.5f));
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1600.f/900.f, 0.1f, 100.0f);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Render the base circle with bass scale
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(bassAmplitude));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(baseCircleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circleSegments + 2);

    // Render the bars
    for (int i = 0; i < numBars; ++i) {
        float angle = (2.0f * M_PI / numBars) * i;
        float radius = 5.0f;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float h = barHeights[i];

        float w = 0.1f; // width of bar
        float d = 0.05f; // depth

        float vertices[] = {
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

        model = glm::mat4(1.0f);
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

static float lastTime = glfwGetTime();
float currentTime = glfwGetTime();
float deltaTime = currentTime - lastTime;
lastTime = currentTime;

    particleSystem.update(deltaTime);
    particleSystem.render();
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

