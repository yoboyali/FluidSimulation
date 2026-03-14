#ifndef FLUIDSIMULATION_INCLUDES_H
#define FLUIDSIMULATION_INCLUDES_H

// OpenGl
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Standard library
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// headers
#include "Observer.h"

// Constants
#define WindowWidth  1500
#define WindowHeight 1080
#define MAX_NEIGHBORS 64

#endif //FLUIDSIMULATION_INCLUDES_H