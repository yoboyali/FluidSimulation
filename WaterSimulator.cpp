//
// Created by Ali Hamdy on 12/02/2026.
//

#include "WaterSimulator.h"

vector2 positions[Numofparticles] = {0};
vector2 velocities[Numofparticles] = {0};

vector2 operator-(vector2 x , vector2 y)
{
    return (vector2){x.x - y.x , x.y - y.y};
}

void WaterSimulator::RenderScene()
{
    SetColor();

    double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    for (int i = 0 ; i < Numofparticles ; i++) {
        double dt = 0;
        velocities[i].y +=  down * gravity * dt;
        CheckCollision(positions[i].y , velocities[i].y , i);
        positions[i].y += velocities[i].y * dt;
      //  std::cout<<"Velocity after: "<<velocities[i].y<<" at index: "<<0<<std::endl;
        DrawSphere(positions[i].x , positions[i].y , 0.0);


    }
    glutSwapBuffers();
    SetColor();
    double temp =  CalculateDensity(positions[150]);
    std::cout<<"Calculated density: "<<temp<<std::endl;
    glPushMatrix();
    glTranslated(positions[170].x , positions[170].y , 0);
    glutWireSphere(smoothingradius , 50 , 50);
    glPopMatrix();
        t2 = t;

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
   // std::cout<<"Final Position: "<<y<<std::endl;
    glTranslated(x , y , z);
    const GLdouble kRadius = radius  ;
    const GLint kLongitudeSlices = 50;
    const GLint kLatitudeStacks = 50;
    glutSolidSphere(kRadius, kLongitudeSlices, kLatitudeStacks);

    glPopMatrix();
}

void WaterSimulator::CalculateVelocities()
{
}

double WaterSimulator::CalculateDensity(vector2 samplepoint)
{
    double density = 0.0;
    const double mass = 1.0;

    for (int i = 0 ; i < Numofparticles ; i++) {
        double dst = magnitude(positions[i] - samplepoint);
        double influence = SmoothingKernel(smoothingradius , dst);
        density += mass * influence;
    }
    return density;
}

double WaterSimulator::SmoothingKernel(double smoothingradius , double dst)
{
    double volume = M_PI * pow(smoothingradius ,8) / 4;
    double value = 0 > smoothingradius *smoothingradius - dst * dst ? 0 : smoothingradius * smoothingradius - dst * dst;
    return value * value * value / volume;
}

void WaterSimulator::CheckCollision(double y , double velocity , int index)
{
        double border = 1 - radius;
        if (y <= -border) {
            positions[index].y = -border;
            velocities[index].y *= -1 * damping;
          //  std::cout<<"collision index:"<<index<<std::endl;
        }
        if (y >= border) {
            positions[index].y = border;
            velocities[index].y *= -1 * damping;
    }

}

void WaterSimulator::start()
{
}

WaterSimulator::WaterSimulator()
{
    int ParticlesperRow = (int)sqrt(Numofparticles);
    int ParticlesperCol = (Numofparticles - 1) / ParticlesperRow + 1;
    float spacing = radius * 2 + particlespacing;
    for (int i = 0 ; i < Numofparticles ; i++) {
        float x = ((i % ParticlesperRow - ParticlesperRow / 2 + 0.5) * spacing) / 10;
        float y = ((i / ParticlesperRow - ParticlesperCol / 2 + 0.5) * spacing) / 10;
      //  std::cout<<"x: "<<x<<" y: "<<y<<std::endl;
        positions[i] = (vector2){x ,y};
    }
}
