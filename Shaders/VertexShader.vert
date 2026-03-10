#version 460 core

layout(std430, binding = 0) buffer PositionBuffer {
	vec4 positions[];
};
layout(std430, binding = 4) buffer ColorBuffer {
	vec4 Colors[];
};

uniform mat4 proj;
uniform mat4 view;
uniform float radius;

out vec2 localPos;
flat out vec3 particleColor;

void main() {
	uint particleIndex = gl_VertexID / 6;
	uint cornerIndex   = gl_VertexID % 6;

	vec2 corners[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2( 1.0,  1.0),
	vec2(-1.0, -1.0),
	vec2( 1.0,  1.0),
	vec2(-1.0,  1.0)
	);

	vec2 corner = corners[cornerIndex];

	vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 camUp    = vec3(view[0][1], view[1][1], view[2][1]);

	vec3 worldPos = positions[particleIndex].xyz
	+ camRight * corner.x * radius
	+ camUp    * corner.y * radius;

	localPos       = corner;
	particleColor  = Colors[particleIndex].rgb;
	gl_Position    = proj * view * vec4(worldPos, 1.0);
}