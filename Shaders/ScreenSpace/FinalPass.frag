#version 460 core
#define saturate(x) clamp(x, 0.0, 1.0)

in vec2 texCoord;

uniform sampler2D thicknessTex;
uniform sampler2D backgroundTex;
uniform sampler2D Tex;
uniform mat4  proj;
uniform float texelSize;
uniform float maxDepth;
uniform float absorptionScale;
uniform vec3 absorptionCoeff;
uniform float xBorder;
uniform float yBorder;
uniform float zBorder;
uniform float refractionStrength;
uniform vec3 fluidColor;
uniform float specularStrength;

vec3 boundsSize = vec3(xBorder, yBorder, zBorder);
vec3 lightDir = normalize(vec3(0.5, 1.0, 1.0));

const float iorAir   = 1.0;
const float iorFluid = 1.3;

out vec4 FragColor;
//Credit Sebastian lague

struct LightResponse {
    vec3  reflectDir;
    vec3  refractDir;
    float reflectWeight;
    float refractWeight;
};

float hermite(float x, float minEdge, float maxEdge) {
    float t = clamp((x - minEdge) / (maxEdge - minEdge), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

vec3 uvToEye(vec2 uv, float depth) {
    mat4  invProj = inverse(proj);
    vec2  ndc     = uv * 2.0 - 1.0;
    vec4  clipPos = vec4(ndc, depth * 2.0 - 1.0, 1.0);
    vec4  viewPos = invProj * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec3 getEyePos(vec2 coord) {
    float depth = texture(Tex, coord).r;
    return uvToEye(coord, depth);
}

vec3 calculateNormals(float depth) {
    vec3 posEye = uvToEye(texCoord, depth);

    vec3 ddx  = getEyePos(texCoord + vec2(texelSize, 0.0))  - posEye;
    vec3 ddx2 = posEye - getEyePos(texCoord - vec2(texelSize, 0.0));
    if (abs(ddx.z) > abs(ddx2.z)) ddx = ddx2;

    vec3 ddy  = getEyePos(texCoord + vec2(0.0, texelSize))  - posEye;
    vec3 ddy2 = posEye - getEyePos(texCoord - vec2(0.0, texelSize));
    if (abs(ddy2.z) < abs(ddy.z)) ddy = ddy2;

    return normalize(cross(ddx, ddy));
}

vec3 CalculateClosestFaceNormal(vec3 boxSize, vec3 p) {
    vec3 halfSize = boxSize * 0.5;
    vec3 o = halfSize - abs(p);
    return (o.x < o.y && o.x < o.z) ? vec3(sign(p.x), 0, 0)
    : (o.y < o.z)               ? vec3(0, sign(p.y), 0)
    : vec3(0, 0, sign(p.z));
}

vec3 SmoothEdgeNormals(vec3 normal, vec3 pos, vec3 boxSize) {
    vec3  o           = boxSize / 2.0 - abs(pos);
    float faceWeight  = max(0.0, min(o.x, o.z));
    vec3  faceNormal  = CalculateClosestFaceNormal(boxSize, pos);
    const float smoothDst = 0.01;
    float cornerWeight = 1.0 - saturate(abs(o.x - o.z) * 6.0);
    faceWeight = 1.0 - hermite(faceWeight, 0.0, smoothDst);
    faceWeight *= (1.0 - cornerWeight);
    return normalize(normal * (1.0 - faceWeight) + faceNormal * faceWeight);
}

float CalculateReflectance(vec3 inDir, vec3 normal, float iorA, float iorB) {
    float refractRatio          = iorA / iorB;
    float cosAngleIn            = -dot(inDir, normal);
    float sinSqrAngleRefraction = refractRatio * refractRatio * (1.0 - cosAngleIn * cosAngleIn);
    if (sinSqrAngleRefraction >= 1.0) return 1.0;

    float cosAngleRefraction = sqrt(1.0 - sinSqrAngleRefraction);
    float rPerp = (iorA * cosAngleIn - iorB * cosAngleRefraction) / (iorA * cosAngleIn + iorB * cosAngleRefraction);
    float rPar  = (iorB * cosAngleIn - iorA * cosAngleRefraction) / (iorB * cosAngleIn + iorA * cosAngleRefraction);
    return (rPerp * rPerp + rPar * rPar) / 2.0;
}

vec3 Refract(vec3 inDir, vec3 normal, float iorA, float iorB) {
    float refractRatio          = iorA / iorB;
    float cosAngleIn            = -dot(inDir, normal);
    float sinSqrAngleRefraction = refractRatio * refractRatio * (1.0 - cosAngleIn * cosAngleIn);
    if (sinSqrAngleRefraction > 1.0) return vec3(0.0);
    return refractRatio * inDir + (refractRatio * cosAngleIn - sqrt(1.0 - sinSqrAngleRefraction)) * normal;
}

LightResponse CalculateReflectionAndRefraction(vec3 inDir, vec3 normal, float iorA, float iorB) {
    LightResponse r;
    r.reflectWeight = CalculateReflectance(inDir, normal, iorA, iorB);
    r.refractWeight = 1.0 - r.reflectWeight;
    r.reflectDir    = reflect(inDir, normal);
    r.refractDir    = Refract(inDir, normal, iorA, iorB);
    return r;
}

vec3 sampleEnvironment(vec3 dir) {
        vec3 envColor = vec3(
        exp(-absorptionCoeff.x),
        exp(-absorptionCoeff.y),
        exp(-absorptionCoeff.z)
        );
        float t = dir.y;
        return mix(envColor , envColor, t);

}

vec3 sampleBackground(vec2 uv) {
    return texture(backgroundTex, uv).rgb;
}
float fresnel(vec3 viewDir, vec3 normal, float ior) {
    float r0 = pow((1.0 - ior) / (1.0 + ior), 2.0);
    return r0 + (1.0 - r0) * pow(1.0 - max(0.0, dot(viewDir, normal)), 5.0);
}
void main() {

    float depth = texture(Tex, texCoord).r;
    if (depth >= maxDepth) {
        FragColor = texture(backgroundTex, texCoord);
        return; }

    vec3 hitPos  = uvToEye(texCoord, depth);
    vec3 viewDir = normalize(-hitPos);
    vec3 normal  = calculateNormals(depth);
    normal       = SmoothEdgeNormals(normal, hitPos, boundsSize);

    LightResponse lr = CalculateReflectionAndRefraction(-viewDir, normal, iorAir, iorFluid);


    float specular = pow(max(0.0, dot(lr.reflectDir, lightDir)), 256.0) * specularStrength;

    vec3 reflectCol = sampleEnvironment(lr.reflectDir);
    reflectCol+= vec3(specular);

    float thickness = texture(thicknessTex , texCoord).r * absorptionScale;

    //frensel apro
    float fresnelWeight = fresnel(viewDir, normal, iorFluid);

    vec2 refractUV  = texCoord + normal.xy * refractionStrength * texelSize;
    vec3 refractCol = sampleBackground(refractUV);

    // Beer's law
    vec3 transmittance = vec3(
    exp(-thickness * absorptionCoeff.x),
    exp(-thickness * absorptionCoeff.y),
    exp(-thickness * absorptionCoeff.z)
    );
    refractCol = refractCol * transmittance;

    vec3 finalColor = mix(refractCol, reflectCol, fresnelWeight);

    FragColor = vec4(finalColor , 1.0);

}