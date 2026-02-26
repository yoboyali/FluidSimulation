//
// Created by Ali Hamdy on 12/02/2026.
//

#ifndef WATERSIMULATOR_H
#define WATERSIMULATOR_H
#include <GL/freeglut.h>
#include <random>
#define Numofparticles 2000

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
    float ViscositySmoothingKernel(float dst);
    vector2 CalculateViscosityForce(int index);
    double magnitude(vector2 a)
    {
        return sqrt((a.x * a.x) + (a.y * a.y));
    }
    static float dot(vector2 a , vector2 b)
    {
        return a.x * b.x + a.y * b.y;
    }
    vector2 down = {0.0 , -1.0};
    const double radius= 0.01 , gravity = 1.0 , damping = 0.1 , particlespacing = 0.3 , smoothingradius = 0.07 , mass = 0.5 , ViscosityStrength = 0.15;
    double oldTime = 0.0;
    float Poly6ScalingFactor;
    double targetDensity = 400.0;
    double pressureMultiplier = 500.70;
    SpatialHash* spatialHash;
public:
    WaterSimulator();
    ~WaterSimulator();
    void RenderScene();
    static void MouseCallBack(int button , int state , int x , int y);




};



#endif //WATERSIMULATOR_H
