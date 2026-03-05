cmake_minimum_required(VERSION 3.31)
project(FluidSimulation)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

# Cross-platform freeglut
if(WIN32)
    set(FREEGLUT_DIR "${CMAKE_SOURCE_DIR}/freeglut")
    set(GLUT_LIBRARY "${FREEGLUT_DIR}/lib/x64/libfreeglut.a")
    set(GLUT_INCLUDE_DIR "${FREEGLUT_DIR}/include")
else()
    find_package(GLUT REQUIRED)
    set(GLUT_INCLUDE_DIR /opt/homebrew/include)
endif()

add_executable(FluidSimulation
        main.cpp
        WaterSimulator.cpp
        WaterSimulator.h
        SpatialHash.cpp
        SpatialHash.h
        vector.cpp
        vector.h)

target_include_directories(FluidSimulation PRIVATE
        ${GLUT_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/glm   # points to the glm folder in your project root
)

target_link_libraries(FluidSimulation
        OpenGL::GL
        ${GLUT_LIBRARY}
        glu32
)