#version 460 core

uniform float xBorder;
uniform float yBorder;
uniform float zBorder;
uniform mat4 proj;
uniform mat4 view;

void main(){

    vec3 positions[8] = vec3[](
    vec3(-xBorder, -yBorder, -zBorder),
    vec3( xBorder, -yBorder, -zBorder),
    vec3( xBorder,  yBorder, -zBorder),
    vec3(-xBorder,  yBorder, -zBorder),
    vec3(-xBorder, -yBorder,  zBorder),
    vec3( xBorder, -yBorder,  zBorder),
    vec3( xBorder,  yBorder,  zBorder),
    vec3(-xBorder,  yBorder,  zBorder)
    );
    int edges[24] = int[](
    0, 1,  1, 2,  2, 3,  3, 0,
    4, 5,  5, 6,  6, 7,  7, 4,
    0, 4,  1, 5,  2, 6,  3, 7
    );

    gl_Position = proj * view * vec4(positions[edges[gl_VertexID]] , 1.0);
}
