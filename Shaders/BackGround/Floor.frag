#version 460 core

in vec2 uv;
out vec4 FragColor;

void main() {
    vec3 purple = vec3(0.8,  0.07, 0.9);
    vec3 yellow = vec3(0.9,  0.9,  0.07);
    vec3 green  = vec3(0.5,  0.92, 0.07);
    vec3 blue   = vec3(0.08, 0.55, 0.92);
    vec3 black = vec3(0.0);
    vec3 white = vec3(1.0);
    vec2 tile   = floor(uv);
    int   ix    = int(mod(tile.x, 2.0));
    int   iy    = int(mod(tile.y, 2.0));

    float offsets[4] = float[] (
    float(-0.3),
    float(0.6),
    float (0.2),
    float(-0.8)
    );
    vec3 palette[4] = vec3[](black + 0.2 ,white - 0.5 , white - 0.5, black + 0.2);


    int   check = ix + iy * 2;
    vec3  col   = palette[check];

    FragColor = vec4(col, 1.0);
}