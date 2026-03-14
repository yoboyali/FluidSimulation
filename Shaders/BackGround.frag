#version 460 core

out vec4 FragColor;
uniform vec2 resolution;

void main(){
    float t = gl_FragCoord.y / resolution.y;
    vec3 top = vec3(0.1 , 0.6 , 0.9);
    vec3 bottom = vec3(1.0 , 1.0 , 1.0);
    FragColor = vec4(mix(bottom , top , t) , 1.0);
}