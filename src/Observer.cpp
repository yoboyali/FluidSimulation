//
// Created by Ali Hamdy on 29/11/2025.
//

#include "Observer.h"
#define MouseSens 0.1f

Observer::Observer(glm::vec3 pos)
    : position(pos)
{
    UpdateVectors();
}
glm::mat4 Observer::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}
void Observer::UpdateVectors()
{
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);

    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}
void Observer::MoveForward(float delta)
{
    position += front * delta;
}

void Observer::MoveRight(float delta)
{
    position += right * delta;
}

void Observer::MoveUp(float delta)
{
    position += worldUp * delta;
}
void Observer::Rotate(float yawOffset, float pitchOffset)
{
    yawOffset *= MouseSens;
    pitchOffset *= MouseSens;

    yaw   += yawOffset;
    pitch += pitchOffset;


    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    UpdateVectors();
}
void Observer::LookAt(const glm::vec3& target)
{
    front = glm::normalize(target - position);
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}

const glm::vec3& Observer::GetPosition() const
{
    return position;
}

