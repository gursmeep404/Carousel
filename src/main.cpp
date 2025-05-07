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

    window = glfwCreateWindow(1600, 900, "Carousel", nullptr, nullptr);
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
    setupBaseCircle();
    renderBaseCircle();
    loadAudio("../assets/willow.ogg");
    initOpenAL();
    playAudio();

    while (!glfwWindowShouldClose(window)) {
    processAudioFrame();
    float time = glfwGetTime();

    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    renderScene();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

    cleanup();
    return 0;
}

