//
// Created by Ali Hamdy on 12/02/2026.
//

#ifndef WATERSIMULATOR_H
#define WATERSIMULATOR_H
#include <GL/freeglut.h>
#include <random>
#define Numofparticles 3000

#include "SpatialHash.h"
#include "vector.h"
class WaterSimulator {



private:
    void SetColor(vector3 color);
    void SetMaterialColor(const GLfloat material_color[4]);
    void DrawSphere(GLdouble x , GLdouble y , GLdouble z);
    void CheckCollision(vector2 position , int index);
    void updateDensities();
    double CalculateDensity(int index);
    double SmoothingKernel(double smoothingradius , double dst);
    double SmoothingKernelDerivative(float smoothingradius , double dst);
    vector2 calculatePressureForce(int index);
    float ConvertDensityToPressure(double density);
    float CalculateSharedPressure(float densityA , float DensityB);
    vector3 EvaluateGradient(float t);
    void renderInfoText();

    double magnitude(vector2 a)
    {
        return sqrt((a.x * a.x) + (a.y * a.y));
    }
    vector2 down = {0 , -1};
    const double radius= 0.01 , gravity = 0.0 , damping = 0.8 , particlespacing = 0.3 , smoothingradius = 0.05 , mass = 0.5;
    double oldTime = 0;
    double targetDensity = 0.3;
    double pressureMultiplier = 200.70;
    SpatialHash* spatialHash;

public:
    WaterSimulator();
    void RenderScene();
    ~WaterSimulator();


};



#endif //WATERSIMULATOR_H
