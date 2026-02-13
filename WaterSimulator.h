//
// Created by Ali Hamdy on 12/02/2026.
//

#ifndef WATERSIMULATOR_H
#define WATERSIMULATOR_H
#include <GL/freeglut.h>
#include <iostream>



class WaterSimulator {

public: void RenderScene();
    void SetColor();
    void SetMaterialColor(const GLfloat material_color[4]);
    void DrawSphere(GLdouble x , GLdouble y , GLdouble z);
    void CalculateVelocities();
    void CalculateDensity();
    void SmoothingKernel();
};



#endif //WATERSIMULATOR_H
