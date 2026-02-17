//
// Created by Ali Hamdy on 12/02/2026.
//

#ifndef WATERSIMULATOR_H
#define WATERSIMULATOR_H
#include <GL/freeglut.h>
#include <iostream>
#include <random>
#define Numofparticles 300

struct vector2
{
    float x , y;
};
class WaterSimulator {



public: void RenderScene();
    void SetColor();
    void SetMaterialColor(const GLfloat material_color[4]);
    void DrawSphere(GLdouble x , GLdouble y , GLdouble z);
    void CalculateVelocities();
    double CalculateDensity(vector2 samplepoint);
    double SmoothingKernel(double smoothingradius , double dst);
    double SmoothingKernelDerivative(float smoothingradius , double dst);
    void CheckCollision(vector2 position , int index);
    void start();
    void updateDensities();
    vector2 calculatePressureForce(int index);
    float ConvertDensityToPressure(double density);
    float CalculateSharedPressure(float densityA , float DensityB);
    double magnitude(vector2 a)
    {
        return sqrt((a.x * a.x) + (a.y * a.y));
    }
    WaterSimulator();
    void drawarrow(vector2 start , vector2 end);
    vector2 down = {0 , -1};
    double radius= 0.02 , gravity = 10.0 , damping = 0.8 , particlespacing = 0.09 , smoothingradius = 0.1 , mass = 1.0;
    double t2 = 0;
    double targetDensity = 2.5;
    double pressureMultiplier = 550.70;

};



#endif //WATERSIMULATOR_H
