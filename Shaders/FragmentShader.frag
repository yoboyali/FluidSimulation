#version 460 core

in vec2 localPos;
flat in vec3 particleColor;
out vec4 FragColor;

uniform mat4 proj;

void main() {
    float dist = length(localPos);
    if (dist > 1.0) discard;

    vec3 normal = normalize(vec3(localPos, sqrt(1.0 - dist * dist)));

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));
    float diffuse  = max(dot(normal, lightDir), 0.0);
    float ambient  = 0.3;
    float specular = pow(max(dot(reflect(-lightDir, normal), vec3(0,0,1)), 0.0), 32.0) * 0.5;

    vec3 color = particleColor * (ambient + diffuse) + vec3(specular);
    FragColor  = vec4(color, 1.0);
}