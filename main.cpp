#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "WaterSimulator.h"
#define WindowWidth  1500
#define WindowHeight 1000
#define NUM_PARTICLES 2000

WaterSimulator simulator;
GLFWwindow* window;

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
GLuint ColSSBO;

glm::mat4 proj;
glm::vec4 particleColor;

float mass             = 0.5;
float smoothingRadius  = 0.07;
float targetDensity    = 400.0;
float pressureMultiplier = 500.70;
float viscosityStrength = 0.15;
float PARTICLE_RADIUS = 0.01f;
float Particlespacing = 0.3f;

float oldTime = 0.0;
bool paused = false;

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
    std::vector<glm::vec4> Colors(NUM_PARTICLES, glm::vec4(0.0f));


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

    glGenBuffers(1, &ColSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ColSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), Colors.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);

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

    float time = glfwGetTime();
    float dt = paused ? 0.0f : time - oldTime;

    glUseProgram(compute_predict);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glUniform1f(glGetUniformLocation(compute_predict, "dt"), dt);
    glUniform2f(glGetUniformLocation(compute_predict, "gravity"), 0.0f, -1.0f);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_density);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);
    glUniform1ui(glGetUniformLocation(compute_density, "NUM_PARTICLES"), NUM_PARTICLES);
    glUniform1f(glGetUniformLocation(compute_density, "dt"), dt);
    glUniform1f(glGetUniformLocation(compute_density, "mass"), mass);
    glUniform1f(glGetUniformLocation(compute_density, "smoothingRadius"), smoothingRadius);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_force);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
    glUniform1ui(glGetUniformLocation(compute_force, "NUM_PARTICLES"), NUM_PARTICLES);
    glUniform1f(glGetUniformLocation(compute_force, "dt"), dt);
    glUniform1f(glGetUniformLocation(compute_force, "mass"), mass);
    glUniform1f(glGetUniformLocation(compute_force, "smoothingRadius"), smoothingRadius);
    glUniform1f(glGetUniformLocation(compute_force, "targetDensity"), targetDensity);
    glUniform1f(glGetUniformLocation(compute_force, "pressureMultiplier"), pressureMultiplier);
    glUniform1f(glGetUniformLocation(compute_force, "viscosityStrength"), viscosityStrength);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_apply);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);
    glUniform1f(glGetUniformLocation(compute_apply, "dt"), dt);
    glDispatchCompute(NUM_PARTICLES / 64 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"),   1, GL_FALSE, glm::value_ptr(proj));
    glUniform1f       (glGetUniformLocation(shaderProgram, "radius"), PARTICLE_RADIUS);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, NUM_PARTICLES * 6);
    oldTime = time;

}

void resetSimulation() {
    init();
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WindowWidth, WindowHeight, "Simulation!", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, WindowWidth, WindowHeight);
    init();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");


    while (!glfwWindowShouldClose(window)) {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(WindowWidth + 10, +20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Always);
        ImGui::Begin("Settings");

        ImGui::SliderFloat("mass", &mass, 0.0f, 5.0f);
        ImGui::SliderFloat("Smoothing Radius", &smoothingRadius, 0.0f, 1.0f);
        ImGui::SliderFloat("Target Density", &targetDensity, 0.0f, 1000.0f);
        ImGui::SliderFloat("Pressure Multiplier", &pressureMultiplier, 0.0f, 500.0f);
        ImGui::SliderFloat("Viscosity Strength", &viscosityStrength , 0.0f, 1.0f);
        ImGui::SliderFloat("Particle Radius", &PARTICLE_RADIUS , 0.0f, 0.6f);
        ImGui::SliderFloat("Particle Spacing", &Particlespacing , 0.0f, 1.0f);
        if (ImGui::Button("Reset Simulation")) {
            resetSimulation();
        }
        ImGui::SameLine();

        const char* Text = paused ? "PAUSED" : "RUNNING";
        if (ImGui::Button(Text)) {
            paused = !paused;
        }
        if (ImGui::Button("Close Simulation")) {
            break;
        }
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        ImGui::End();
            display();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    glfwTerminate();
    return 0;
}