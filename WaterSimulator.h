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
    void CheckCollision(double y , double velocity , int index);
    void start();
    double calculateProperty(vector2 samplepoint);
    vector2 calculatePropertyGradient(vector2 samplepoint);
    double magnitude(vector2 a)
    {
        return sqrt((a.x * a.x) + (a.y * a.y));
    }
    WaterSimulator();
    void drawarrow(vector2 start , vector2 end);
    float ExampleFunction(vector2 pos);
    double radius= 0.01 , gravity = 00.0 , down = -1.0 , damping = 0.5 , particlespacing = 0.5 , smoothingradius = 0.2;
    double t2 = 0;

};



#endif //WATERSIMULATOR_H
