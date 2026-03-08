#version 460 core

layout(std430, binding = 0) buffer PositionBuffer {
	vec2 positions[];
};

uniform mat4 proj;
uniform float radius;

out vec2 localPos;

void main() {
	uint particleIndex = gl_VertexID / 6;  // 6 vertices per particle
	uint cornerIndex   = gl_VertexID % 6;  // which vertex of the 2 triangles

	// two triangles forming a quad
	vec2 corners[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2( 1.0,  1.0),
	vec2(-1.0, -1.0),
	vec2( 1.0,  1.0),
	vec2(-1.0,  1.0)
	);

	vec2 center = positions[particleIndex];
	vec2 offset = corners[cornerIndex] * radius;

	localPos = corners[cornerIndex];  // passes -1 to 1 range to fragment shader
	gl_Position = proj * vec4(center + offset, 0.0, 1.0);
}