#include "renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <kiss_fft.h>
#include <iostream>
#include "shaders.h"
#include "audio.h"
#include "cleanup.h" 

extern GLuint shaderProgram;


const int fftSize = 512;
const int numBars = fftSize / 2;
GLuint barVAO[numBars], barVBO[numBars];
float barHeights[numBars] = {0};

extern GLFWwindow* window;


kiss_fft_cfg fftCfg;
kiss_fft_cpx* fftInput;
kiss_fft_cpx* fftOutput;


void initOpenGL() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        exit(-1);
    }
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

void cleanup()
{
    if (fftCfg) free(fftCfg);
    if (fftInput) free(fftInput);
    if (fftOutput) free(fftOutput);

    for (int i = 0; i < numBars; ++i) {
        glDeleteVertexArrays(1, &barVAO[i]);
        glDeleteBuffers(1, &barVBO[i]);
    }
    
    if (audioData) free(audioData);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}
    

