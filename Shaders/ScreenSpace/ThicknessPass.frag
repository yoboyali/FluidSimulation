#version 460 core

in vec2 texCoord;
out float thickness;

void main() {
    float r2 = dot(texCoord, texCoord);
    if (r2 > 1.0) discard;

    thickness = exp(-r2 * 2.0);
}
