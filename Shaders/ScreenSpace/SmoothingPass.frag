#version 460 core

uniform sampler2D Tex;
uniform float blurScale;
uniform float filterRadius;
uniform vec2  blurDir;
uniform float blurDepthFalloff;
in vec2 texCoord;
out float smoothedDepth;


void main() {
    float depth = texture(Tex, texCoord).r;
    if (depth >= 0.9999) {
        smoothedDepth = depth;
        return;
    }

    float adaptiveRadius  = clamp(filterRadius * (1.0 / max(abs(depth), 0.1)), 1.0, filterRadius);
    float adaptiveScale   = 1.0 / (adaptiveRadius * 0.5);
    float adaptiveFalloff = clamp(blurDepthFalloff / max(abs(depth), 0.1), blurDepthFalloff, blurDepthFalloff * 20.0);

    float sum  = 0.0;
    float wsum = 0.0;

    for (float i = -adaptiveRadius; i <= adaptiveRadius; i += 1.0) {
        float Sample = texture(Tex, texCoord + i * blurDir).r;

        float r = i * blurScale;
        float w = exp(-r * r);

        float r2 = (Sample - depth) * adaptiveFalloff;
        float g  = exp(-r2 * r2);

        sum  += Sample * w * g;
        wsum += w * g;
    }

    smoothedDepth = (wsum > 0.0) ? sum / wsum : depth;
}