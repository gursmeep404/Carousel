#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <kiss_fft.h>
#define STB_VORBIS_IMPLEMENTATION
#include "../external/stb/stb_vorbis.c"


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
    loadAudio("../assets/willow.ogg");

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


short* audioData = nullptr;  // Pointer to audio data (using short instead of int)
int audioDataSize = 0;       // Size of the audio data
int sampleRate = 0;

void loadAudio(const char* filename) {
    int channels;

    // Call the stb_vorbis function to decode the audio
    audioDataSize = stb_vorbis_decode_filename(filename, &channels, &sampleRate, &audioData);

    if (audioData == nullptr) {
        std::cerr << "Failed to load audio\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Audio loaded successfully\n";
}



kiss_fft_cfg fftCfg;
kiss_fft_cpx* fftInput;
kiss_fft_cpx* fftOutput;
int fftSize = 1024; // FFT size, adjust this for resolution of frequency data

void processAudio() {
    // Allocate memory for FFT input and output buffers
    fftInput = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * fftSize);
    fftOutput = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * fftSize);

    // Prepare the FFT input with audio data (taking a window of data for FFT)
    for (int i = 0; i < fftSize; ++i) {
        if (i < audioDataSize) {
            fftInput[i].r = audioData[i]; // Real part
            fftInput[i].i = 0;            // Imaginary part
        } else {
            fftInput[i].r = 0;
            fftInput[i].i = 0;
        }
    }

    // Perform FFT on the audio data
    kiss_fft(fftCfg, fftInput, fftOutput);

    // Process the frequency data (fftOutput contains complex frequency data)
    for (int i = 0; i < fftSize; ++i) {
        // Use fftOutput[i].r (real part) or fftOutput[i].i (imaginary part) as frequency data
        float magnitude = sqrt(fftOutput[i].r * fftOutput[i].r + fftOutput[i].i * fftOutput[i].i);
        // Use 'magnitude' to update your visuals (for example, make bars react to the magnitude)
    }

    // Free memory after processing
    free(fftInput);
    free(fftOutput);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // You can use the frequency data from the FFT to create dynamic visuals
    // For example, create bars for different frequency bands
    float barWidth = 0.1f;  // Set the width of each bar
    for (int i = 0; i < fftSize; ++i) {
        float magnitude = fftOutput[i].r;  // Get the magnitude (you can also use fftOutput[i].i for imaginary part)
        float height = magnitude * 0.01f; // Scale the magnitude to the size of the bar

        GLfloat vertices[] = {
            -0.9f + i * barWidth, -0.5f, 0.0f,
            -0.9f + i * barWidth, -0.5f + height, 0.0f,
            -0.9f + (i + 1) * barWidth, -0.5f + height, 0.0f,
            -0.9f + (i + 1) * barWidth, -0.5f, 0.0f
        };

        GLuint barVAO, barVBO;
        glGenVertexArrays(1, &barVAO);
        glGenBuffers(1, &barVBO);

        glBindVertexArray(barVAO);
        glBindBuffer(GL_ARRAY_BUFFER, barVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(barVAO);
        glDrawArrays(GL_QUADS, 0, 4);  // Draw the bar

        glBindVertexArray(0);
    }

    glBindVertexArray(0);
}



void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
fftCfg = kiss_fft_alloc(fftSize, 0, nullptr, nullptr);  // Allocate FFT setup
free(fftCfg);
free(audioData);


}


