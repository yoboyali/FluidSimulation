#version 460 core

in vec2 texCoord;
in vec3 eyeSpacePos;
flat in vec4 color;

uniform float radius;
uniform mat4 proj;

out vec4 FragColor;

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));
    vec3 N;
    N.xy = texCoord;
    float r2 = dot(N.xy , N.xy);
    if(r2 > 1.0){ discard; }
    N.z = -sqrt(1.0 - r2 );

    vec4 pixelPos = vec4(eyeSpacePos + N * radius , 1.0);
    vec4 clipPos  = proj * pixelPos;
    gl_FragDepth = clipPos.z / clipPos.w;

    float linearDepth = (clipPos.w - 0.01) / (100.0 - 0.01);
    float expDepth = 1.0 - exp(-linearDepth * 10.0);
    float ambient = 0.5;

    vec3 Color = vec3(expDepth) + ambient;
    FragColor = vec4(Color, 1.0);
}