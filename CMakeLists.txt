cmake_minimum_required(VERSION 3.31)
project(FluidSimulation)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

if(WIN32)
    set(FREEGLUT_DIR "${CMAKE_SOURCE_DIR}/freeglut")
    set(GLUT_LIBRARY "${FREEGLUT_DIR}/lib/x64/libfreeglut.a")
    set(GLUT_INCLUDE_DIR "${FREEGLUT_DIR}/include")
    set(GLFW_DIR "${CMAKE_SOURCE_DIR}/glfw")
    set(GLFW_INCLUDE_DIR "${GLFW_DIR}/include")
    set(GLFW_LIBRARY "${GLFW_DIR}/lib-mingw-w64/libglfw3.a")

else()
    find_package(GLUT REQUIRED)
    set(GLUT_INCLUDE_DIR /opt/homebrew/include)
    find_package(glfw3 REQUIRED)
    set(GLFW_LIBRARY glfw)
    set(GLFW_INCLUDE_DIR /opt/homebrew/include)
endif()

add_executable(FluidSimulation
        src/main.cpp
        src/Observer.h
        src/Observer.cpp
        glad/src/glad.c
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_impl_glfw.cpp
        imgui/imgui_impl_opengl3.cpp
        src/Fluid.cpp
        src/Fluid.h
        src/includes.h
)

target_include_directories(FluidSimulation PRIVATE
        ${GLFW_INCLUDE_DIR}
        ${GLUT_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/glad/include
        ${CMAKE_SOURCE_DIR}/glm
        ${CMAKE_SOURCE_DIR}/imgui
)

target_link_libraries(FluidSimulation
        OpenGL::GL
        ${GLUT_LIBRARY}
        glu32
        ${GLFW_LIBRARY}
)