cmake_minimum_required(VERSION 3.31)
project(FluidSimulation)

set(CMAKE_CXX_STANDARD 20)


find_package(OpenGL REQUIRED)

find_library(GLUT_LIBRARY glut PATHS /opt/homebrew/lib REQUIRED)

add_executable(FluidSimulation main.cpp)

target_include_directories(FluidSimulation PRIVATE /opt/homebrew/include)

target_link_libraries(FluidSimulation
        ${OPENGL_gl_LIBRARY}
        ${GLUT_LIBRARY}
)

