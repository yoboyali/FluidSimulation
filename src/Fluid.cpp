#include "Fluid.h"

void Fluid::Init() {

    RecalculateConstants();
    std::vector<int>       cellEntries(numParticles , 0);
    std::vector<int>       queryIds(numParticles , 0);
    std::vector<int>       cellStart(tableSize + 1 , 0);

    std::vector<glm::vec4> positions(numParticles);
    std::vector<glm::vec4> velocities(numParticles, glm::vec4(0.0f));
    std::vector<glm::vec2> Densities(numParticles, glm::vec2(0.0f));
    std::vector<glm::vec4> PredictedPositions(numParticles, glm::vec4(0.0f));
    std::vector<glm::vec4> Colors(numParticles, glm::vec4(0.0 , 0.0 , 1.0 , 0.0));

    int ParticlesperRow    = (int)cbrt(numParticles);
    int ParticlesperCol    = ParticlesperRow;
    int ParticlesperDepth  = (numParticles - 1) / (ParticlesperRow * ParticlesperCol) + 1;
    float spacing          = particleRadius * 2 + particleSpacing;

    for (int i = 0; i < numParticles; i++) {
        //if (i > NUM_PARTICLES / 2){spawnOffsetX =  -xBorder * 0.5f;}
        float x = ((i % ParticlesperRow) - ParticlesperRow / 2.0f + 0.5f) * spacing;
        float y = ((i / ParticlesperRow) % ParticlesperCol - ParticlesperCol / 2.0f + 0.5f) * spacing;
        float z = ((i / (ParticlesperRow * ParticlesperCol)) - ParticlesperDepth / 2.0f + 0.5f) * spacing;
        positions[i] = glm::vec4(x, y, z , 0.0);
    }

    glGenBuffers(1, &posSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(glm::vec4), positions.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);

    glGenBuffers(1, &velSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(glm::vec4), velocities.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);

    glGenBuffers(1, &predSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, predSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(glm::vec4), PredictedPositions.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);

    glGenBuffers(1, &densSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(glm::vec2), Densities.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);

    glGenBuffers(1, &ColSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ColSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(glm::vec4), Colors.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);

    glGenBuffers(1, &cellStartSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellStartSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (tableSize + 1) * sizeof(int), cellStart.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);

    glGenBuffers(1, &cellEntriesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellEntriesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(int), cellEntries.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);

    glGenBuffers(1, &queryIdsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, queryIdsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(int), queryIds.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, queryIdsSSBO);

    glGenBuffers(1, &neighborListSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborListSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * MAX_NEIGHBORS * sizeof(int), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, neighborListSSBO);

    glGenBuffers(1, &neighborCountSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighborCountSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numParticles * sizeof(int), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, neighborCountSSBO);

    std::vector<int> cellOffset(tableSize + 1, 0);
    glGenBuffers(1, &cellOffsetSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellOffsetSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (tableSize + 1) * sizeof(int), cellOffset.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, cellOffsetSSBO);


    glGenVertexArrays(1, &VAO);

    shaderProgram       = CreateShaderProgram("Shaders/VertexShader.vert", "Shaders/FragmentShader.frag");
    compute_predict     = CreateComputeProgram("ComputeShaders/Predicted.comp");
    compute_density     = CreateComputeProgram("ComputeShaders/Density.comp");
    compute_force       = CreateComputeProgram("ComputeShaders/Force.comp");
    compute_apply       = CreateComputeProgram("ComputeShaders/Apply.comp");
    compute_hashCount   = CreateComputeProgram("ComputeShaders/HashCount.comp");
    compute_hashBuild   = CreateComputeProgram("ComputeShaders/HashBuild.comp");
    compute_hashReset   = CreateComputeProgram("ComputeShaders/HashReset.comp");

    proj = glm::perspective(glm::radians(60.0f),(float)WindowWidth / (float)WindowHeight, 0.01f, 100.0f);

}

void Fluid::Render(glm::mat4 view) {


    float time   = glfwGetTime();
    float fullDt = paused ? 0.0f : time - oldTime;
    float dt     = fullDt / simulationSteps;

    dt = std::min(dt, 0.005f);

    CreateImGuiWindow();

    for (int i = 0; i < simulationSteps ; i++) {
        glUseProgram(compute_predict);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
        glUniform1f(glGetUniformLocation(compute_predict, "dt"), dt);
        glUniform1f(glGetUniformLocation(compute_predict, "gravity"), gravity);
        glUniform3f(glGetUniformLocation(compute_predict, "down"), 0.0f, -1.0f , 0.0);
        glUniform1ui(glGetUniformLocation(compute_predict, "NUM_PARTICLES"), numParticles);
        glDispatchCompute(numParticles / 1024 + 1,  1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glUseProgram(compute_hashReset);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
        glUniform1ui(glGetUniformLocation(compute_hashReset, "tableSize"), tableSize);
        glDispatchCompute((tableSize + 1) / 1024 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glUseProgram(compute_hashCount);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, queryIdsSSBO);
        glUniform1ui(glGetUniformLocation(compute_hashCount, "NUM_PARTICLES"), numParticles);
        glUniform1f(glGetUniformLocation(compute_hashCount, "Spacing"), smoothingRadius);
        glUniform1ui(glGetUniformLocation(compute_hashCount, "tableSize") , tableSize);
        glDispatchCompute(numParticles / 1024 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        BuildHash();

        glUseProgram(compute_density);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, neighborListSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, neighborCountSSBO);
        glUniform1ui(glGetUniformLocation(compute_density, "NUM_PARTICLES"), numParticles);
        glUniform1f(glGetUniformLocation(compute_density, "dt"), dt);
        glUniform1f(glGetUniformLocation(compute_density, "mass"), mass);
        glUniform1f(glGetUniformLocation(compute_density, "smoothingRadius"), smoothingRadius);
        glUniform1f(glGetUniformLocation(compute_density, "Spacing"), smoothingRadius);
        glUniform1ui(glGetUniformLocation(compute_density, "tableSize") , tableSize);
        glUniform1f(glGetUniformLocation(compute_density , "K_SpikyPow2") , K_SpikyPow2);
        glUniform1f(glGetUniformLocation(compute_density , "K_SpikyPow3") , K_SpikyPow3);
        glDispatchCompute(numParticles / 1024 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glUseProgram(compute_force);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cellStartSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, cellEntriesSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, neighborListSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, neighborCountSSBO);
        glUniform1ui(glGetUniformLocation(compute_force, "NUM_PARTICLES"), numParticles);
        glUniform1f(glGetUniformLocation(compute_force, "dt"), dt);
        glUniform1f(glGetUniformLocation(compute_force, "mass"), mass);
        glUniform1f(glGetUniformLocation(compute_force, "smoothingRadius"), smoothingRadius);
        glUniform1f(glGetUniformLocation(compute_force, "targetDensity"), targetDensity);
        glUniform1f(glGetUniformLocation(compute_force, "pressureMultiplier"), pressureMultiplier);
        glUniform1f(glGetUniformLocation(compute_force, "viscosityStrength"), viscosityStrength);
        glUniform1f(glGetUniformLocation(compute_force, "Spacing"), smoothingRadius);
        glUniform1ui(glGetUniformLocation(compute_force, "tableSize") , tableSize);
        glUniform1f(glGetUniformLocation(compute_force, "K_SpikyPow2Grad") , K_SpikyPow2Grad);
        glUniform1f(glGetUniformLocation(compute_force, "K_SpikyPow3Grad") , K_SpikyPow3Grad);
        glUniform1f(glGetUniformLocation(compute_force, "nearPressureMultiplier") , nearPressureMultiplier);

        glDispatchCompute(numParticles / 1024 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glUseProgram(compute_apply);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, densSSBO);
        glUniform1f(glGetUniformLocation(compute_apply, "dt"), dt);
        glUniform1ui(glGetUniformLocation(compute_apply, "NUM_PARTICLES"), numParticles);
        glUniform1f(glGetUniformLocation(compute_apply , "xBorder") , xBorder);
        glUniform1f(glGetUniformLocation(compute_apply , "yBorder") , yBorder);
        glUniform1f(glGetUniformLocation(compute_apply , "zBorder") , zBorder);
        glUniform1f(glGetUniformLocation(compute_apply, "targetDensity"), targetDensity);
        glDispatchCompute(numParticles / 1024 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ColSSBO);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(glGetUniformLocation(shaderProgram, "radius"), particleRadius);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, numParticles * 6);
        oldTime = time;
}

void Fluid::BuildHash() {

    std::vector<int> cellStartCPU(tableSize + 1);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellStartSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,(tableSize + 1) * sizeof(int),cellStartCPU.data());

    for (int i = 1; i <= tableSize; i++) {
        cellStartCPU[i] += cellStartCPU[i - 1];
    }
    std::vector<int> cellOffsetCPU(cellStartCPU);
    for (int i = tableSize; i > 0; i--) {
        cellStartCPU[i] = cellStartCPU[i - 1];
    }
    cellStartCPU[0] = 0;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellStartSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,(tableSize + 1) * sizeof(int),cellStartCPU.data());

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellOffsetSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,(tableSize + 1) * sizeof(int),cellOffsetCPU.data());

    glUseProgram(compute_hashBuild);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2,  predSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5,  cellStartSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6,  cellEntriesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, cellOffsetSSBO);
    glUniform1ui(glGetUniformLocation(compute_hashBuild, "NUM_PARTICLES"), numParticles);
    glUniform1f(glGetUniformLocation(compute_hashBuild,  "Spacing"),       smoothingRadius);
    glUniform1ui(glGetUniformLocation(compute_hashBuild, "tableSize"),     tableSize);
    glDispatchCompute(numParticles / 1024 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

}

void Fluid::ResetScene() {Init();}

void Fluid::CreateImGuiWindow() {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Always);
    ImGui::Begin("Settings");
    ImGui::Text("Number of Particles: %d", numParticles);
    ImGui::SliderFloat("Gravity", &gravity, -10.0f, 10.0f);
    ImGui::SliderFloat("Mass", &mass, 0.0f, 5.0f);
    // ImGui::SliderFloat("Smoothing Radius", &smoothingRadius, 0.0f, 1.0f);
    ImGui::SliderFloat("Target Density", &targetDensity, -100.0f , 100000);
    ImGui::SliderFloat("Pressure Multiplier", &pressureMultiplier, 0.0f, 150.0f);
    ImGui::SliderFloat("Near Pressure Multiplier", &nearPressureMultiplier, 0.0f, 150.0f);
    ImGui::SliderFloat("Viscosity Strength", &viscosityStrength , 0.0f, 1.0f);
    ImGui::SliderFloat("Particle Radius", &particleRadius , 0.0f, 0.6f);
    ImGui::SliderFloat("Particle Spacing", &particleSpacing , 0.0f, 1.0f);
    ImGui::SliderFloat("The X Border", &xBorder , 0.2f, 10.0f);
    ImGui::SliderFloat("The Y Border", &yBorder , 0.2f, 10.0f);
    ImGui::SliderFloat("The Z Border", &zBorder , 0.2f, 10.0f);
    ImGui::SliderInt("Simulation Steps ", &simulationSteps , 1, 5.);
    if (ImGui::Button("Reset Simulation")) {ResetScene();}
    ImGui::SameLine();

    const char* Text = paused ? "PAUSED" : "RUNNING";
    if (ImGui::Button(Text)) {
        paused = !paused;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::Button("Close Simulation")) {glfwSetWindowShouldClose(Window, true);}
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}

void Fluid::RecalculateConstants() {

    float particleDiameter   = particleRadius * 2.0f;

    smoothingRadius          = particleDiameter * 3.5f;

    K_SpikyPow2              = 15.0f / (2.0f * M_PI * pow(smoothingRadius, 5.0f));
    K_SpikyPow3              = 15.0f / (M_PI        * pow(smoothingRadius, 6.0f));
    K_SpikyPow2Grad          = 15.0f / (M_PI        * pow(smoothingRadius, 5.0f));
    K_SpikyPow3Grad          = 45.0f / (M_PI        * pow(smoothingRadius, 6.0f));

    float selfDensity        = mass * smoothingRadius * smoothingRadius * K_SpikyPow2;
    targetDensity            = selfDensity * 6.0f;

}

std::string Fluid::LoadShaderSource(const char *filePath) {

    std::ifstream file(filePath);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();

}

GLuint Fluid::CreateComputeProgram(const char *path) {

    std::string src = LoadShaderSource(path);
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

GLuint Fluid::CreateShaderProgram(const char *vertPath, const char *fragPath) {

    std::string vertSrc = LoadShaderSource(vertPath);
    std::string fragSrc = LoadShaderSource(fragPath);
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

Fluid::Fluid(int NumParticles, GLFWwindow* window) {

    Window = window;
    numParticles = NumParticles;
    tableSize = 2 * numParticles;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    Init();

}

Fluid::~Fluid() {

    glDeleteBuffers(1, &posSSBO);
    glDeleteBuffers(1, &velSSBO);
    glDeleteBuffers(1, &predSSBO);
    glDeleteBuffers(1, &densSSBO);
    glDeleteBuffers(1, &ColSSBO);
    glDeleteBuffers(1, &cellStartSSBO);
    glDeleteBuffers(1, &cellEntriesSSBO);
    glDeleteBuffers(1, &queryIdsSSBO);
    glDeleteBuffers(1, &neighborListSSBO);
    glDeleteBuffers(1, &neighborCountSSBO);
    glDeleteBuffers(1, &cellOffsetSSBO);
    glDeleteVertexArrays(1, &VAO);

    glDeleteProgram(shaderProgram);
    glDeleteProgram(compute_predict);
    glDeleteProgram(compute_density);
    glDeleteProgram(compute_force);
    glDeleteProgram(compute_apply);
    glDeleteProgram(compute_hashCount);
    glDeleteProgram(compute_hashBuild);
    glDeleteProgram(compute_hashReset);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}
