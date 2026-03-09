#include <fstream>
#include <glad/glad.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "WaterSimulator.h"
#define WindowWidth  1500
#define WindowHeight 1000
#define NUM_PARTICLES 1000
#define PARTICLE_RADIUS 0.02f
#define Particlespacing 0.3f

WaterSimulator simulator;

GLuint shaderProgram;

GLuint compute_predict;
GLuint compute_density;
GLuint compute_force;
GLuint compute_apply;

GLuint VAO;
GLuint posSSBO;
GLuint predSSBO;
GLuint velSSBO;
GLuint densSSBO;

glm::mat4 proj;

std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint createShaderProgram(const char* vertPath, const char* fragPath) {
    std::string vertSrc = loadShaderSource(vertPath);
    std::string fragSrc = loadShaderSource(fragPath);
    const char* vSrc = vertSrc.c_str();
    const char* fSrc = fragSrc.c_str();

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vSrc, nullptr);
    glCompileShader(vert);

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fSrc, nullptr);
    glCompileShader(frag);

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

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
    std::vector<glm::vec2> positions(NUM_PARTICLES);
    std::vector<glm::vec2> velocities(NUM_PARTICLES, glm::vec2(0.0f));
    std::vector<glm::vec2> PredictedPositions(NUM_PARTICLES, glm::vec2(0.0f));
    std::vector<glm::vec2> Densities(NUM_PARTICLES, glm::vec2(0.0f));


    int ParticlesperRow = (int) sqrt(NUM_PARTICLES);
    int ParticlesperCol = (NUM_PARTICLES - 1) / ParticlesperRow + 1;
    float spacing = PARTICLE_RADIUS * 2 + Particlespacing;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        float x = ((i % ParticlesperRow - ParticlesperRow / 2 + 0.5) * spacing) / 10;
        float y = ((i / ParticlesperRow - ParticlesperCol / 2 + 0.5) * spacing) / 10;
        positions[i] = glm::vec2(x , y);
    }

    glGenBuffers(1, &posSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec2), positions.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);

    glGenBuffers(1, &velSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec2), velocities.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);

    glGenBuffers(1, &predSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, predSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec2), PredictedPositions.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);

    glGenBuffers(1, &densSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec2), Densities.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);

    glGenVertexArrays(1, &VAO);

    shaderProgram   = createShaderProgram("VertexShader.vert", "FragmentShader.frag");
    compute_predict = createComputeProgram("ComputeShaders/Predicted.comp");
    compute_density = createComputeProgram("ComputeShaders/Density.comp");
    compute_force   = createComputeProgram("ComputeShaders/Force.comp");
    compute_apply   = createComputeProgram("ComputeShaders/Apply.comp");

    proj = glm::ortho(-(float)WindowWidth/WindowHeight,
                       (float)WindowWidth/WindowHeight,
                      -1.0f, 1.0f);
}

void display() {
    static float dt = 0.016f;


    glUseProgram(compute_predict);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glUniform1f(glGetUniformLocation(compute_predict, "dt"), dt);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_density);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, densSSBO);
    glUniform1f(glGetUniformLocation(compute_density, "dt"), dt);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_force);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, densSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velSSBO);
    glUniform1f(glGetUniformLocation(compute_force, "dt"), dt);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_apply);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSBO);
    glUniform1f(glGetUniformLocation(compute_apply, "dt"), dt);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"),   1, GL_FALSE, glm::value_ptr(proj));
    glUniform1f       (glGetUniformLocation(shaderProgram, "radius"), PARTICLE_RADIUS);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, NUM_PARTICLES * 6);

    glutSwapBuffers();
    glutPostRedisplay();
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