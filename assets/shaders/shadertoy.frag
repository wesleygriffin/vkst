#version 450

layout(push_constant) uniform block {
    vec4 iMouse;
    float iTime;
    float iTimeDelta;
    float iFrameRate;
    float iFrame;
    vec3 iResolution;
    float padding0;
    //vec3 iChannelResolution[4];
    //sampler2D iChannel[4];
};

layout(location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 fragColor;

#include "uv.frag"
//#include "tdm_seascape.frag"
//#include "iq_raymarching_primitives.frag"

void main() {
    mainImage(fragColor, fragCoord);
    //fragColor = vec4(0., 0., iTimeDelta * 100, 1.);
    //fragColor = vec4(fragCoord.xy, 0., 1.) / vec4(iResolution.xy, 1., 1.);
}
