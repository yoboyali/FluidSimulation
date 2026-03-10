//
// Created by Ali Hamdy on 29/11/2025.
//

#ifndef OBSERVER_H
#define OBSERVER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>


class Observer
{
public:
    Observer(glm::vec3 position = {0.0f, 0.0f, 3.0f});

    glm::mat4 GetViewMatrix() const;

    void MoveForward(float delta);
    void MoveRight(float delta);
    void MoveUp(float delta);
    void LookAt(const glm::vec3& target);
    void Rotate(float yawOffset, float pitchOffset);
    const glm::vec3& GetPosition() const;
private:
    void UpdateVectors();

    glm::vec3 position;

    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp = {0.0f, 1.0f, 0.0f};

    float yaw   = -90.0f; // looking down -Z
    float pitch = 0.0f;
};



#endif //OBSERVER_H
