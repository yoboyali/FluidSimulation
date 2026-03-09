#version 460 core

in vec2 localPos;
out vec4 FragColor;
flat in vec4 particleColor;
void main() {
    float dist = length(localPos);
    if (dist > 1.0) discard;
    FragColor = vec4(particleColor);
}