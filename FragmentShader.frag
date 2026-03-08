#version 460 core

in vec2 localPos;
out vec4 FragColor;

void main() {
    float dist = length(localPos);
    if (dist > 1.0) discard;
    FragColor = vec4(0.3, 0.6, 1.0, 1.0);
}