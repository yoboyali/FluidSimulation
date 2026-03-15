#ifndef FLUIDSIMULATION_FLUID_H
#define FLUIDSIMULATION_FLUID_H
#include "includes.h"

class Fluid {
    // Vars

    GLFWwindow* Window;

    GLuint shaderProgram ,compute_predict ,compute_density ,compute_force ,compute_apply ,compute_hashCount ,compute_hashBuild ,compute_hashReset;
    GLuint VAO ,posSSBO ,predSSBO ,velSSBO ,densSSBO ,ColSSBO ,cellStartSSBO ,cellEntriesSSBO ,queryIdsSSBO ,neighborListSSBO ,neighborCountSSBO ,cellOffsetSSBO;
    GLuint keysSSBO , compute_keyGen , compute_radixSort , particleKeyOutSSBO ,cellEntriesOutSSBO ,sortedPosSSBO, sortedVelSSBO, sortedPredSSBO , compute_reorder , compute_writeback;
    GLuint depthPass , backGroundShader , boxShader , floorShader , depthTex , depthFBO , depthRBO , normalPass , smoothingPass, blurFBO , blurFBO2;
    GLuint blurTex , blurTex2 ,blurRBO ,blurRBO2;



    float mass                   = 1.0f;
    float gravity                = 10.0f;
    float oldTime                = 0.0f;
    float xBorder                = 4.4f;
    float yBorder                = 2.0f;
    float zBorder                = 1.5f;
    float particleRadius         = 0.015f;
    float particleSpacing        = 0.015f;
    float K_SpikyPow2            = 0.0f;
    float K_SpikyPow3            = 0.0f;
    float K_SpikyPow2Grad        = 0.0f;
    float K_SpikyPow3Grad        = 0.0f;
    float K_Viscosity            = 0.0f;
    float smoothingRadius        = 0.05f;
    float targetDensity          = 500.0f;
    float pressureMultiplier     = 31.0f;
    float viscosityStrength      = 0.15f;
    float nearPressureMultiplier = 45.3f;
    float maxDepth               = 0.0f;
    float blurScale              = -0.027f;
    float filderRadius           = 20.0f;
    float blurDepthFalloff       = 5.0f;

    int tableSize;
    int numParticles;
    int simulationSteps = 1;

    bool paused = true;
    bool showDensity = false;
    bool render = true;
    bool showBackGround = true;

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

public:
    //Public functions
    void Render(glm::mat4 view);
    // Constructors and Destructors
    Fluid(int NumParticles , GLFWwindow* window);
    ~Fluid();

};


#endif //FLUIDSIMULATION_FLUID_H