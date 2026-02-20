//
// Created by Ali Hamdy on 12/02/2026.
//

#include "WaterSimulator.h"

vector2 positions[Numofparticles] = {0};
vector2 velocities[Numofparticles] = {0};
double densities[Numofparticles];



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
        vector2 PressureAcc = (Pressure / densities[i]);
        velocities[i] = velocities[i] + (PressureAcc * dt );
        velocities[i] = velocities[i] + (down * gravity *dt);
        const float velocityDamping = 0.95; // Adjust between 0.95-0.995
        velocities[i] = velocities[i] * velocityDamping;
        positions[i] = (velocities[i] * dt) + positions[i];
        CheckCollision(positions[i] , i);
        DrawSphere(positions[i].x , positions[i].y , 0.9);

    }
    glutSwapBuffers();
    t2 = t;

}

WaterSimulator::~WaterSimulator()
{
    delete spatialHash;
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

double WaterSimulator::CalculateDensity(int index)
{
    double density = 0.0;

    spatialHash->query(positions, index, smoothingradius);

    for (int j = 0; j < spatialHash->querySize; j++) {
        int neighborIndex = spatialHash->queryIds[j];

        double dst = magnitude(positions[index] - positions[neighborIndex]);
        double influence = SmoothingKernel(smoothingradius, dst);
        density += mass * influence;
    }
    return density;
}

double WaterSimulator::SmoothingKernel(double smoothingradius , double dst)
{
    if (dst >= smoothingradius) return 0;
    float volume = (M_PI * pow(smoothingradius , 4)) / 6;
    return (smoothingradius - dst) * (smoothingradius - dst) / volume;
}
double WaterSimulator::SmoothingKernelDerivative(float smoothingradius, double dst)
{
    if (dst >= smoothingradius) return 0;
    float scale = 12 / (pow(smoothingradius , 4) * M_PI);
    return (dst - smoothingradius) * scale;
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
    if (pos.x <= -1.4) {
        positions[index].x = -1.4;
        velocities[index].x *= -1 * damping;
        //  std::cout<<"collision index:"<<index<<std::endl;
    }
    if (pos.x >= 1.4) {
        positions[index].x = 1.4;
        velocities[index].x *= -1 * damping;
    }

}

void WaterSimulator::start()
{
}

void WaterSimulator::updateDensities()
{
    spatialHash->create(positions , Numofparticles);
    for (int i = 0; i < Numofparticles ; i++) {
        densities[i] = CalculateDensity(i);
    }
}

vector2 WaterSimulator::calculatePressureForce(int index)
{
    vector2 PressureForce = {0,0};
    float mass = 1.0;
    spatialHash->query(positions, index, smoothingradius);

    for (int i = 0; i < spatialHash->querySize ; i++) {
        int neighborIndex = spatialHash->queryIds[i];
        if (neighborIndex == index) continue;
        vector2 offset = positions[index] - positions[neighborIndex];
        double dst = magnitude(offset);
        if (dst < 0.0001) continue; // Avoid division by zero

        vector2 dir = offset / dst;
        float slope = SmoothingKernelDerivative(smoothingradius, dst);

        float pressure_i = ConvertDensityToPressure(densities[index]);
        float pressure_j = ConvertDensityToPressure(densities[neighborIndex]);

        // SPH pressure force formula
        float pressure_term = (pressure_i / (densities[index] * densities[index]) +
                               pressure_j / (densities[neighborIndex] * densities[neighborIndex]));

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
    int ParticlesperRow = (int) sqrt(Numofparticles);
    int ParticlesperCol = (Numofparticles - 1) / ParticlesperRow + 1;
    float spacing = radius * 2 + particlespacing;
    spatialHash = new SpatialHash(smoothingradius, Numofparticles);
    std::default_random_engine gen;
    std::uniform_real_distribution<double> xdistribution(-0.9,
                                                         0.9);
    std::uniform_real_distribution<double> ydistribution(-0.9,
                                                         0.9);
    for (int i = 0; i < Numofparticles; i++) {
        //float x = xdistribution(gen);
        //float y = ydistribution(gen);
        float x = ((i % ParticlesperRow - ParticlesperRow / 2 + 0.5) * spacing) / 10;
        float y = ((i / ParticlesperRow - ParticlesperCol / 2 + 0.5) * spacing) / 10;
        //  std::cout<<"x: "<<x<<" y: "<<y<<std::endl;
        positions[i] = (vector2){x, y};
        // properties[i] = ExampleFunction(positions[i]);
    }
}
