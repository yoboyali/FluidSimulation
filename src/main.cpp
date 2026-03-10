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
#define NUM_PARTICLES 20000

WaterSimulator simulator;
GLFWwindow* window;

GLuint shaderProgram;

GLuint compute_predict;
GLuint compute_density;
GLuint compute_force;
GLuint compute_apply;
GLuint compute_hashCount;
GLuint compute_hashBuild;
GLuint compute_hashReset;

GLuint VAO;
GLuint posSSBO;
GLuint predSSBO;
GLuint velSSBO;
GLuint densSSBO;
GLuint ColSSBO;
GLuint cellStartSSBO;
GLuint cellEntriesSSBO;
GLuint queryIdsSSBO;

glm::mat4 proj;
glm::mat4 view = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 5.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);
glm::vec4 particleColor;

float mass             = 1.0;
float smoothingRadius  = 0.1;
float targetDensity    = 2.0;
float pressureMultiplier = 600.0;
float viscosityStrength = 0.15;
float PARTICLE_RADIUS = 0.02f;
float Particlespacing = 0.007f;
float gravity = 0.0;
float oldTime = 0.0;
int tableSize = NUM_PARTICLES * 2;
bool paused = true;

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
    std::vector<glm::vec4> positions(NUM_PARTICLES);
    std::vector<glm::vec4> velocities(NUM_PARTICLES, glm::vec4(0.0f));
    std::vector<glm::vec4> PredictedPositions(NUM_PARTICLES, glm::vec4(0.0f));
    std::vector<float> Densities(NUM_PARTICLES, 0.0f);
    std::vector<glm::vec4> Colors(NUM_PARTICLES, glm::vec4(0.0 , 0.0 , 1.0 , 0.0));
    std::vector<int> cellEntries(NUM_PARTICLES , 0);
    std::vector<int> queryIds(NUM_PARTICLES , 0);
    std::vector<int> cellStart(tableSize + 1 , 0);

    int ParticlesperRow = (int)cbrt(NUM_PARTICLES);
    int ParticlesperCol = ParticlesperRow;
    int ParticlesperDepth = (NUM_PARTICLES - 1) / (ParticlesperRow * ParticlesperCol) + 1;
    float spacing = PARTICLE_RADIUS * 2 + Particlespacing;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        float x = ((i % ParticlesperRow) - ParticlesperRow / 2.0f + 0.5f) * spacing;
        float y = ((i / ParticlesperRow) % ParticlesperCol - ParticlesperCol / 2.0f + 0.5f) * spacing;
        float z = ((i / (ParticlesperRow * ParticlesperCol)) - ParticlesperDepth / 2.0f + 0.5f) * spacing;
        positions[i] = glm::vec4(x, y, z , 0.0);
    }

    glGenBuffers(1, &posSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), positions.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);

    glGenBuffers(1, &velSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), velocities.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);

    glGenBuffers(1, &predSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, predSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), PredictedPositions.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);

    glGenBuffers(1, &densSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(float), Densities.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);

    glGenBuffers(1, &ColSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ColSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), Colors.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);

    glGenBuffers(1, &cellStartSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellStartSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (tableSize + 1) * sizeof(int), cellStart.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);

    glGenBuffers(1, &cellEntriesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellEntriesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(int), cellEntries.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);

    glGenBuffers(1, &queryIdsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, queryIdsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(int), queryIds.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, queryIdsSSBO);


    glGenVertexArrays(1, &VAO);

    shaderProgram   = createShaderProgram("Shaders/VertexShader.vert", "Shaders/FragmentShader.frag");
    compute_predict = createComputeProgram("ComputeShaders/Predicted.comp");
    compute_density = createComputeProgram("ComputeShaders/Density.comp");
    compute_force   = createComputeProgram("ComputeShaders/Force.comp");
    compute_apply   = createComputeProgram("ComputeShaders/Apply.comp");
    compute_hashCount=createComputeProgram("ComputeShaders/HashCount.comp");
    compute_hashBuild=createComputeProgram("ComputeShaders/HashBuild.comp");
    compute_hashReset=createComputeProgram("ComputeShaders/HashReset.comp");

    proj = glm::perspective(glm::radians(60.0f),
                            (float)WindowWidth / (float)WindowHeight,
                            0.01f, 100.0f);

}

void display() {

    float time = glfwGetTime();
    float dt = paused ? 0.0f : time - oldTime;
    glUseProgram(compute_predict);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glUniform1f(glGetUniformLocation(compute_predict, "dt"), dt);
    glUniform1f(glGetUniformLocation(compute_predict, "gravity"), gravity);
    glUniform3f(glGetUniformLocation(compute_predict, "down"), 0.0f, -1.0f , 0.0);
    glUniform1ui(glGetUniformLocation(compute_apply, "NUM_PARTICLES"), NUM_PARTICLES);
    glDispatchCompute(NUM_PARTICLES / 256 + 1,  1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_hashReset);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
    glUniform1ui(glGetUniformLocation(compute_hashReset, "tableSize"), tableSize);
    glDispatchCompute((tableSize + 1) / 256 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_hashCount);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, queryIdsSSBO);
    glUniform1ui(glGetUniformLocation(compute_hashCount, "NUM_PARTICLES"), NUM_PARTICLES);
    glUniform1f(glGetUniformLocation(compute_hashCount, "Spacing"), smoothingRadius);
    glUniform1ui(glGetUniformLocation(compute_hashCount, "tableSize") , tableSize);
    glDispatchCompute(NUM_PARTICLES / 256 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_hashBuild);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);
    glUniform1ui(glGetUniformLocation(compute_hashBuild, "NUM_PARTICLES"), NUM_PARTICLES);
    glUniform1f(glGetUniformLocation(compute_hashBuild, "Spacing"), smoothingRadius);
    glUniform1ui(glGetUniformLocation(compute_hashBuild, "tableSize") , tableSize);
    glDispatchCompute( 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_density);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);
    glUniform1ui(glGetUniformLocation(compute_density, "NUM_PARTICLES"), NUM_PARTICLES);
    glUniform1f(glGetUniformLocation(compute_density, "dt"), dt);
    glUniform1f(glGetUniformLocation(compute_density, "mass"), mass);
    glUniform1f(glGetUniformLocation(compute_density, "smoothingRadius"), smoothingRadius);
    glUniform1f(glGetUniformLocation(compute_density, "Spacing"), smoothingRadius);
    glUniform1ui(glGetUniformLocation(compute_density, "tableSize") , tableSize);
    glDispatchCompute(NUM_PARTICLES / 256 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_force);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);
    glUniform1ui(glGetUniformLocation(compute_force, "NUM_PARTICLES"), NUM_PARTICLES);
    glUniform1f(glGetUniformLocation(compute_force, "dt"), dt);
    glUniform1f(glGetUniformLocation(compute_force, "mass"), mass);
    glUniform1f(glGetUniformLocation(compute_force, "smoothingRadius"), smoothingRadius);
    glUniform1f(glGetUniformLocation(compute_force, "targetDensity"), targetDensity);
    glUniform1f(glGetUniformLocation(compute_force, "pressureMultiplier"), pressureMultiplier);
    glUniform1f(glGetUniformLocation(compute_force, "viscosityStrength"), viscosityStrength);
    glUniform1f(glGetUniformLocation(compute_force, "Spacing"), smoothingRadius);
    glUniform1ui(glGetUniformLocation(compute_force, "tableSize") , tableSize);
    glDispatchCompute(NUM_PARTICLES / 256 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(compute_apply);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);
    glUniform1f(glGetUniformLocation(compute_apply, "dt"), dt);
    glUniform1ui(glGetUniformLocation(compute_apply, "NUM_PARTICLES"), NUM_PARTICLES);
    glDispatchCompute(NUM_PARTICLES / 256 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform1f(glGetUniformLocation(shaderProgram, "radius"), PARTICLE_RADIUS);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, NUM_PARTICLES * 6);
    oldTime = time;

}

void resetSimulation() {
    init();
}

extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WindowWidth + 240 , WindowHeight, "Simulation!", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGL();
    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, WindowWidth, WindowHeight);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, WindowWidth, WindowHeight);
    init();
    std::vector<glm::vec4> debugPos(NUM_PARTICLES);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(glm::vec4), debugPos.data());
    std::cout << "First particle pos: " << debugPos[0].x << " " << debugPos[0].y << " " << debugPos[0].z << std::endl;
    std::cout << "Last  particle pos: " << debugPos[NUM_PARTICLES-1].x << " " << debugPos[NUM_PARTICLES-1].y << " " << debugPos[NUM_PARTICLES-1].z << std::endl;

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
        ImGui::SetNextWindowPos(ImVec2(WindowWidth + 80 , 100), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Always);
        ImGui::Begin("Settings");
        ImGui::Text("Number of Particles: %d" , NUM_PARTICLES);
        ImGui::SliderFloat("Gravity", &gravity, -10.0f, 10.0f);
        ImGui::SliderFloat("Mass", &mass, 0.0f, 5.0f);
        ImGui::SliderFloat("Smoothing Radius", &smoothingRadius, 0.0f, 1.0f);
        ImGui::SliderFloat("Target Density", &targetDensity, 0.0f, 1000.0f);
        ImGui::SliderFloat("Pressure Multiplier", &pressureMultiplier, 0.0f, 1500.0f);
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
    return 0;
}