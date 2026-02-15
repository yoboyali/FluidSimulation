//
// Created by Ali Hamdy on 12/02/2026.
//

#ifndef WATERSIMULATOR_H
#define WATERSIMULATOR_H
#include <GL/freeglut.h>
#include <iostream>
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
    void CheckCollision(double y , double velocity , int index);
    void start();
    double magnitude(vector2 a)
    {
        return sqrt((a.x * a.x) + (a.y * a.y));
    }
    WaterSimulator();
    double radius= 0.05 , gravity = 10.0 , down = -1.0 , damping = 0.5 , particlespacing = 1.0 , smoothingradius = 0.2;
    double t2 = 0;

};



#endif //WATERSIMULATOR_H
