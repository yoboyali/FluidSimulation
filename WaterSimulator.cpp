//
// Created by Ali Hamdy on 12/02/2026.
//

#include "WaterSimulator.h"


void WaterSimulator::RenderScene()
{


    static GLdouble x = 0.0;
    static GLdouble y = 0.0;
    static double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

    GLdouble u = -(x - 0.5 * cos(t * t) * cos(t));
    GLdouble v = -(y - 0.5 * cos(t * t) * sin(t));

    double t_plus_dt = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    double dt = t_plus_dt - t;

    x += dt * u;  // x(t + dt) = x(t) + dt * u(x, y, z, t)
    y += dt * v;

    t = t_plus_dt;
    SetColor();
    DrawSphere(x , y ,0.0);

    glutSwapBuffers();
}

void WaterSimulator::SetColor()
{
    const GLfloat kRedColor[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    SetMaterialColor(kRedColor);
}

void WaterSimulator::SetMaterialColor(const GLfloat material_color[4])
{
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_color);
}

void WaterSimulator::DrawSphere(GLdouble x , GLdouble y , GLdouble z)
{
    glPushMatrix();
    glTranslated(x , y , z);
    const GLdouble kRadius = 0.1  ;
    const GLint kLongitudeSlices = 50;
    const GLint kLatitudeStacks = 50;
    glutSolidSphere(kRadius, kLongitudeSlices, kLatitudeStacks);

    glPopMatrix();
}

void WaterSimulator::CalculateVelocities()
{
}

void WaterSimulator::CalculateDensity()
{
}

void WaterSimulator::SmoothingKernel()
{
}
