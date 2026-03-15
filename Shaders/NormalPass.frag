#version 460 core

uniform sampler2D depthTex;
uniform mat4 proj;
uniform float texelSize;
uniform float maxDepth;


in vec2 texCoord;
out vec4 FragColor;

vec3 uvToEye(vec2 uv, float depth) {
    mat4 invProj = inverse(proj);
    vec2 ndc     = uv * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = invProj * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec3 getEyePos(sampler2D tex, vec2 coord) {
    float depth = texture(tex, coord).r;
    return uvToEye(coord, depth);
}

void main() {
    float depth = texture(depthTex, texCoord).r;
    if (depth >= maxDepth) {
        discard;
    }

    vec3 posEye = uvToEye(texCoord, depth);

    vec3 ddx  = getEyePos(depthTex, texCoord + vec2(texelSize, 0.0)) - posEye;
    vec3 ddx2 = posEye - getEyePos(depthTex, texCoord - vec2(texelSize, 0.0));
    if (abs(ddx.z) > abs(ddx2.z)) ddx = ddx2;

    vec3 ddy  = getEyePos(depthTex, texCoord + vec2(0.0, texelSize)) - posEye;
    vec3 ddy2 = posEye - getEyePos(depthTex, texCoord - vec2(0.0, texelSize));
    if (abs(ddy2.z) < abs(ddy.z)) ddy = ddy2;

    vec3 n = normalize(cross(ddx, ddy));

        FragColor = vec4(n * 0.5 + 0.5, 1.0);
}