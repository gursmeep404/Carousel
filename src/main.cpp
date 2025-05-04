#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <kiss_fft.h>
#include "audio.h"
#include "renderer.h"
#include "shaders.h"
#include "cleanup.h"


GLFWwindow* window;


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
    initOpenAL();
    playAudio();

    while (!glfwWindowShouldClose(window)) {
        processAudioFrame();
        renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}

