#version 450

layout(push_constant) uniform block {
    vec4 iMouse;
    float iTime;
    float iTimeDelta;
    float iFrameRate;
    int iFrame;
    vec3 iResolution;
    //vec3 iChannelResolution[4];
    //sampler2D iChannel[4];
};

layout(location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 fragColor;

#include "uv.frag"
//#include "tdm_seascape.frag"
//#include "iq_raymarching_primitives.frag"

void main() {
    //mainImage(fragColor, fragCoord);
    //fragColor = vec4(0., 0., float(iTimeDelta) * .5, 1.);
    fragColor = vec4(fragCoord.xy, 0., 1.);// / iResolution.xy;
    //fragColor = vec4(uv, 0., 1.);
}
