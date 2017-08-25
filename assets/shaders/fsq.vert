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

layout(location = 0) out vec2 fragCoord;

void main() {
    fragCoord = vec2((gl_VertexIndex << 1) & 2, (gl_VertexIndex & 2));
    gl_Position = vec4(fragCoord * 2.0 - 1.0, 0.f, 1.0);
    fragCoord.y *= -1;
    fragCoord.y += 1;
    //fragCoord *= iResolution.xy;
}
