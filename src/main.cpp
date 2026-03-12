#include "Fluid.h"
#include "includes.h"

int particleNumber = 100000;

GLFWwindow* window;

Observer observer;
Observer* gObserver = nullptr;

bool firstMouse     = true;
bool mouseCaptured  = true;

float lastX = 400.0f, lastY  = 300.0f;

glm::mat4 view;

extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;
}
void MouseCallback(GLFWwindow* Window, double xPosition, double yPosition) {
    if (!mouseCaptured) return;

    if (firstMouse) {
        lastX = (float)xPosition;
        lastY = (float)yPosition;
        firstMouse = false;
    }

    float xOffset = (float)xPosition - lastX;
    float yOffset = lastY - (float)yPosition;

    lastX = (float)xPosition;
    lastY = (float)yPosition;
    gObserver->Rotate(xOffset, yOffset);
}
void CheckUserInput() {
    static bool tabWasPressed = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !tabWasPressed)
        {mouseCaptured = !mouseCaptured;glfwSetInputMode(window, GLFW_CURSOR,mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
        tabWasPressed = true;
    }

    if (glfwGetKey(window, GLFW_KEY_W)            == GLFW_PRESS) observer.MoveForward(0.05f);
    if (glfwGetKey(window, GLFW_KEY_S)            == GLFW_PRESS) observer.MoveForward(-0.05f);
    if (glfwGetKey(window, GLFW_KEY_A)            == GLFW_PRESS) observer.MoveRight(-0.05f);
    if (glfwGetKey(window, GLFW_KEY_D)            == GLFW_PRESS) observer.MoveRight(0.05f);
    if (glfwGetKey(window, GLFW_KEY_SPACE)        == GLFW_PRESS) observer.MoveUp(0.05f);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE)       == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)   == GLFW_PRESS) observer.MoveUp(-0.05f);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) tabWasPressed = false;

    gObserver = &observer;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //window = glfwCreateWindow(WindowWidth, WindowHeight, "Simulation!", NULL, NULL);
    //glfwMakeContextCurrent(window);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    window = glfwCreateWindow(mode->width, mode->height, "Title", monitor, nullptr);
    glfwMakeContextCurrent(window);

    observer = Observer(glm::vec3(3.0f, 3.0f, 3.0f));
    observer.LookAt(glm::vec3(0.0f));
    gObserver = &observer;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseCallback);

    gladLoadGL();
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, WindowWidth, WindowHeight);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, WindowWidth, WindowHeight);

    view = observer.GetViewMatrix();
    Fluid sim(particleNumber, window);

    while (!glfwWindowShouldClose(window)) {
        view = observer.GetViewMatrix();
        sim.Render(view);
        glfwSwapBuffers(window);
        CheckUserInput();
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}