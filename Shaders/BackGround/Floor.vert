#version 460 core

uniform float xBorder;
uniform float yBorder;
uniform float zBorder;
uniform float floorScale;
uniform mat4 proj;
uniform mat4 view;

out vec2 uv;

void main() {
    vec3 positions[6] = vec3[](
    vec3(-xBorder * floorScale, -yBorder, -xBorder * floorScale),
    vec3( xBorder * floorScale, -yBorder, -xBorder * floorScale),
    vec3( xBorder * floorScale, -yBorder,  xBorder * floorScale),
    vec3( xBorder * floorScale, -yBorder,  xBorder * floorScale),
    vec3(-xBorder * floorScale, -yBorder,  xBorder * floorScale),
    vec3(-xBorder * floorScale, -yBorder, -xBorder * floorScale)
    );

    vec2 uvs[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0)
    );

    uv = uvs[gl_VertexID] * floorScale * 0.2;
    gl_Position = proj * view * vec4(positions[gl_VertexID], 1.0);
}