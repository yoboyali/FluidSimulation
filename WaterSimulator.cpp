//
// Created by Ali Hamdy on 12/02/2026.
//

#include "WaterSimulator.h"

vector2 positions[Numofparticles] = {0};
vector2 velocities[Numofparticles] = {0};
double properties[Numofparticles];
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

    double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    for (int i = 0 ; i < Numofparticles ; i++) {
        double dt = t - t2;
        velocities[i].y +=  down * gravity * dt;
        positions[i].y += velocities[i].y * dt;
        CheckCollision(positions[i].y , velocities[i].y , i);

      //  std::cout<<"Velocity after: "<<velocities[i].y<<" at index: "<<0<<std::endl;
        DrawSphere(positions[i].x , positions[i].y , 0.0);
        vector2 pos =calculatePropertyGradient(positions[i]);
        drawarrow(positions[i] , pos);

    }
    glutSwapBuffers();
    SetColor();
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
    double volume = M_PI * pow(smoothingradius ,8.0) / 4.0;
    double value = 0 > smoothingradius *smoothingradius - dst * dst ? 0 : smoothingradius * smoothingradius - dst * dst;
    return value * value * value / volume;
}

double WaterSimulator::SmoothingKernelDerivative(float smoothingradius, double dst)
{
    if (dst >= smoothingradius){return 0.0;}
    float f = smoothingradius * smoothingradius - dst * dst;
    float scale = -24.0 / (M_PI * pow(smoothingradius , 8.0));
   // std::cout<<"scale: "<<scale * dst * f * f<<std::endl;
    return scale * dst * f * f;

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

double WaterSimulator::calculateProperty(vector2 samplepoint)
{
    double property = 0.0;
    double mass = 1.0;
    for (int i = 0 ; i < Numofparticles ; i++) {
        float dst = magnitude(positions[i] - samplepoint);
        float influence = SmoothingKernel(smoothingradius , dst);
        float density = CalculateDensity(positions[i]);
        property += properties[i] * mass / density * influence ;
    }
    return property;
}

vector2 WaterSimulator::calculatePropertyGradient(vector2 samplepoint)
{
  vector2 Gradient = {0,0};
    float mass = 1;
    for (int i = 0 ; i < Numofparticles ; i++) {
        float dst = magnitude(positions[i] - samplepoint);
        if (positions[i] == samplepoint){continue;}
        vector2 dir = (positions[i] - samplepoint) / dst;
        float slope = SmoothingKernelDerivative(smoothingradius , dst);
        float density = CalculateDensity(positions[i]);
        std::cout<<"Slope: "<<slope<<" density: "<<density<<std::endl;
        Gradient =  ((dir * properties[i] * slope * mass /density) + Gradient);
    }
  //  std::cout<<"End of arrow x: "<<Gradient.x<<" y: "<<Gradient.y<<std::endl;

    return  Gradient * -1;
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

        float x = xdistribution(gen);
        float y = ydistribution(gen);
       // float x = ((i % ParticlesperRow - ParticlesperRow / 2 + 0.5) * spacing) / 10;
        //float y = ((i / ParticlesperRow - ParticlesperCol / 2 + 0.5) * spacing) / 10;
      //  std::cout<<"x: "<<x<<" y: "<<y<<std::endl;
        positions[i] = (vector2){x ,y};
        properties[i] = ExampleFunction(positions[i]);
    }
}

void WaterSimulator::drawarrow(vector2 start, vector2 end)
{
    glBegin(GL_LINES);
    end = start + end * 0.1;
    glVertex2f(start.x, start.y);
    glVertex2f(end.x, end.y);
    glEnd();
    //std::cout<<"Start of arrow x: "<<start.x<<" y: "<<start.y<<std::endl;
    //std::cout<<"End of arrow x: "<<end.x<<" y: "<<end.y<<std::endl;

    // Calculate angle for the arrowhead
    float angle = atan2(end.y - start.y, end.x - start.x);
    float headSize = 0.05f; // Size of arrow head

    // Draw the arrowhead (triangle)
    glBegin(GL_TRIANGLES);
    glVertex2f(end.x, end.y); // Point of the arrow
    glVertex2f(end.x - headSize * cos(angle - 0.5), end.y - headSize * sin(angle - 0.5));
    glVertex2f(end.x - headSize * cos(angle + 0.5), end.y - headSize * sin(angle + 0.5));
    glEnd();
}

float WaterSimulator::ExampleFunction(vector2 pos)
{
    return cos(pos.y - 3 + sin(pos.x));
}

