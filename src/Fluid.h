#ifndef FLUIDSIMULATION_FLUID_H
#define FLUIDSIMULATION_FLUID_H
#include "includes.h"

class Fluid {
    // Vars

    GLFWwindow* Window;

    GLuint shaderProgram ,compute_predict ,compute_density ,compute_force ,compute_apply ,compute_hashCount ,compute_hashBuild ,compute_hashReset;
    GLuint VAO ,posSSBO ,predSSBO ,velSSBO ,densSSBO ,ColSSBO ,cellStartSSBO ,cellEntriesSSBO ,queryIdsSSBO ,neighborListSSBO ,neighborCountSSBO ,cellOffsetSSBO;

    float mass                   = 1.0f;
    float gravity                = 10.0f;
    float oldTime                = 0.0f;
    float xBorder                = 3.4f;
    float yBorder                = 2.0f;
    float zBorder                = 1.0f;
    float particleRadius         = 0.015f;
    float particleSpacing        = 0.015f;
    float K_SpikyPow2            = 0.0f;
    float K_SpikyPow3            = 0.0f;
    float K_SpikyPow2Grad        = 0.0f;
    float K_SpikyPow3Grad        = 0.0f;
    float smoothingRadius        = 0.05f;
    float targetDensity          = 500.0f;
    float pressureMultiplier     = 31.0f;
    float viscosityStrength      = 0.15f;
    float nearPressureMultiplier = 31.3f;

    int tableSize;
    int numParticles;
    int simulationSteps = 1;

    bool paused = true;
    bool showDensity = false;

    glm::mat4 proj;
    glm::vec4 particleColor;

    // Functions

    void Init();
    void BuildHash();
    void ResetScene();
    void CreateImGuiWindow();
    void RecalculateConstants();

    std::string LoadShaderSource(const char *filePath);

    GLuint CreateComputeProgram(const char *path);
    GLuint CreateShaderProgram(const char *vertPath , const char *fragPath);

    // Constructors and Destructors
public:
    void Render(glm::mat4 view);
    Fluid(int NumParticles , GLFWwindow* window);
    ~Fluid();

};


#endif //FLUIDSIMULATION_FLUID_H