//
// Created by Ali Hamdy on 12/02/2026.
//

#include "WaterSimulator.h"

vector2 positions[Numofparticles] = {0};
vector2 velocities[Numofparticles] = {0};
double densities[Numofparticles];

vector2 operator-(vector2 x , vector2 y)
{
    return (vector2){x.x - y.x , x.y - y.y};
}
bool operator==(vector2 a , vector2 b)
{
    if (a.x == b.x && a.y == b.y){return true;}
    else
        return false;
}
vector2 operator+(vector2 a , vector2 b)
{
    return (vector2){a.x + b.x , a.y + b.y};
}
vector2 operator+(vector2 b ,float a )
{
    return (vector2){b.x + a, a + b.y};
}
vector2 operator*(vector2 a , vector2 b)
{
    return (vector2){a.x * b.x , a.y * b.y};
}
vector2 operator*(vector2 a , float b)
{
    return (vector2){a.x * b , a.y * b};
}vector2 operator/(vector2 a , float b)
{
    return (vector2){a.x / b , a.y / b};
}

void WaterSimulator::RenderScene()
{
    SetColor();
    updateDensities();

    double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    double dt = t - t2;
    // Cap the time step for stability
    const double maxDt = 0.005; // 5ms maximum
    if (dt > maxDt) dt = maxDt;
    for (int i = 0 ; i < Numofparticles ; i++) {
        vector2 Pressure =calculatePressureForce(i);
        vector2 PressureAcc = Pressure / densities[i];
        velocities[i] = velocities[i] + (PressureAcc * dt);
        velocities[i] = velocities[i] + (down * gravity * dt);
        const float velocityDamping = 0.95; // Adjust between 0.95-0.995
        velocities[i] = velocities[i] * velocityDamping;
        positions[i] = (velocities[i] * dt) + positions[i];
        CheckCollision(positions[i] , i);
        DrawSphere(positions[i].x , positions[i].y , 0.0);
      //  drawarrow(positions[i] , pos);

    }
    glutSwapBuffers();
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
    const GLint kLongitudeSlices = 8;
    const GLint kLatitudeStacks = 8;
    glutSolidSphere(kRadius, kLongitudeSlices, kLatitudeStacks);

    glPopMatrix();
}

void WaterSimulator::CalculateVelocities()
{
}

double WaterSimulator::CalculateDensity(vector2 samplepoint)
{
    double density = 0.0;

    for (int i = 0 ; i < Numofparticles ; i++) {

        double dst = magnitude(positions[i] - samplepoint);
        double influence = SmoothingKernel(smoothingradius , dst);
        density += mass * influence;

    }
    return density;
}

double WaterSimulator::SmoothingKernel(double smoothingradius , double dst)
{
    double volume = M_PI * pow(smoothingradius ,8.0) / 4.0;
    double value = 0 > smoothingradius *smoothingradius - dst * dst ? 0 : smoothingradius * smoothingradius - dst * dst;
    return value * value * value / volume;
}
double WaterSimulator::SmoothingKernelDerivative(float smoothingradius, double dst)
{
    if (dst >= smoothingradius || dst < 0.0001) return 0.0;
    float h = smoothingradius;
    float r = dst;
    float volume = M_PI * pow(h, 8.0) / 4.0;
    return -6.0 * r * pow(h*h - r*r, 2.0) / volume;
}

void WaterSimulator::CheckCollision(vector2 pos ,int index)
{
        double border = 1 - radius;
        if (pos.y <= -border) {
            positions[index].y = -border;
            velocities[index].y *= -1 * damping;
          //  std::cout<<"collision index:"<<index<<std::endl;
        }
        if (pos.y >= border) {
            positions[index].y = border;
            velocities[index].y *= -1 * damping;
    }
    if (pos.x <= -border) {
        positions[index].x = -border;
        velocities[index].x *= -1 * damping;
        //  std::cout<<"collision index:"<<index<<std::endl;
    }
    if (pos.x >= border) {
        positions[index].x = border;
        velocities[index].x *= -1 * damping;
    }

}

void WaterSimulator::start()
{
}

void WaterSimulator::updateDensities()
{
    for (int i = 0; i < Numofparticles ; i++) {
        densities[i] = CalculateDensity(positions[i]);
    }
}

vector2 WaterSimulator::calculatePressureForce(int index)
{
    vector2 PressureForce = {0,0};
    float mass = 1.0;

    for (int i = 0; i < Numofparticles; i++) {
        if (i == index) continue;

        vector2 offset = positions[index] - positions[i];
        double dst = magnitude(offset);
        if (dst < 0.0001) continue; // Avoid division by zero

        vector2 dir = offset / dst;
        float slope = SmoothingKernelDerivative(smoothingradius, dst);

        float pressure_i = ConvertDensityToPressure(densities[index]);
        float pressure_j = ConvertDensityToPressure(densities[i]);

        // SPH pressure force formula
        float pressure_term = (pressure_i / (densities[index] * densities[index]) +
                               pressure_j / (densities[i] * densities[i]));

        PressureForce = PressureForce - dir * mass * mass * pressure_term * slope;
    }
    return PressureForce;
}

float WaterSimulator::ConvertDensityToPressure(double density)
{
    double densityError = density - targetDensity;
    double pressure = densityError * pressureMultiplier;
    return pressure;
}

float WaterSimulator::CalculateSharedPressure(float densityA, float densityB)
{
    float pressureA = ConvertDensityToPressure(densityA);
    float pressureB = ConvertDensityToPressure(densityB);
    return (pressureA + pressureB) / 2;
}

WaterSimulator::WaterSimulator()
{
    int ParticlesperRow = (int)sqrt(Numofparticles);
    int ParticlesperCol = (Numofparticles - 1) / ParticlesperRow + 1;
    float spacing = radius * 2 + particlespacing;

    std::default_random_engine gen;
    std::uniform_real_distribution<double> xdistribution(-0.9,
                                                   0.9);
    std::uniform_real_distribution<double> ydistribution(-0.9,
                                                  0.9);
    for (int i = 0 ; i < Numofparticles ; i++) {

        //float x = xdistribution(gen);
        //float y = ydistribution(gen);
        float x = ((i % ParticlesperRow - ParticlesperRow / 2 + 0.5) * spacing) / 10;
        float y = ((i / ParticlesperRow - ParticlesperCol / 2 + 0.5) * spacing) / 10;
      //  std::cout<<"x: "<<x<<" y: "<<y<<std::endl;
        positions[i] = (vector2){x ,y};
       // properties[i] = ExampleFunction(positions[i]);
    }
}




