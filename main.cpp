#include <fstream>
#include <glad/glad.h>
#include <GL/freeglut.h>

#include <iostream>
#include <sstream>

#include "WaterSimulator.h"
#define WindowWidth 1500
#define WindowHeight 1000
WaterSimulator simulator;

std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Helper to compile and link shaders
GLuint createShaderProgram(const char* vertPath, const char* fragPath) {
    // Load source
    std::string vertSrc = loadShaderSource(vertPath);
    std::string fragSrc = loadShaderSource(fragPath);
    const char* vSrc = vertSrc.c_str();
    const char* fSrc = fragSrc.c_str();

    // Compile vertex shader
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vSrc, nullptr);
    glCompileShader(vert);

    // Compile fragment shader
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fSrc, nullptr);
    glCompileShader(frag);

    // Link into program
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

GLuint shaderProgram;
GLuint computeProgram;
GLuint screenTexture;
GLuint VAO, VBO;

// fullscreen quad
float vertices[] = {
   // pos              // uvs
   -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
    1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
   -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
   -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
};

GLuint createComputeProgram(const char* path) {
    std::string src = loadShaderSource(path);
    const char* cSrc = src.c_str();
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &cSrc, nullptr);
    glCompileShader(shader);
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    glDeleteShader(shader);
    return program;
}

void init() {
    // create texture for compute shader to write to
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // setup fullscreen quad
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // compile shaders
    shaderProgram = createShaderProgram("VertexShader.vert", "FragmentShader.frag");
    computeProgram = createComputeProgram("ComputeShader.comp");
}

void display() {
    // 1. run compute shader to write to texture
    glUseProgram(computeProgram);
    glBindImageTexture(0, screenTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(WindowWidth / 16, WindowHeight / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // wait for compute to finish

    // 2. render fullscreen quad using the texture
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WindowWidth, WindowHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Simulation!");

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WindowWidth, WindowHeight);
    init();

    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}