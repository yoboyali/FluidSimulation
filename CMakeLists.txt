cmake_minimum_required(VERSION 3.31)
project(FluidSimulation)

set(CMAKE_CXX_STANDARD 20)


find_package(OpenGL REQUIRED)

find_library(GLUT_LIBRARY glut PATHS /opt/homebrew/lib REQUIRED)

find_package(glm REQUIRED)
find_path(GLM_INCLUDE_DIR glm/glm.hpp)

add_executable(FluidSimulation main.cpp
        WaterSimulator.cpp
        WaterSimulator.h)

target_include_directories(FluidSimulation PRIVATE /opt/homebrew/include)
target_include_directories(FluidSimulation PRIVATE include  ${GLM_INCLUDE_DIR})

target_link_libraries(FluidSimulation
        ${OPENGL_gl_LIBRARY}
        ${GLUT_LIBRARY}
)

