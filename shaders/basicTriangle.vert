#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vCol;
layout(location = 2) in vec3 vNorm;
layout(location = 3) in vec2 vTex;

layout(location = 0) out vec4 fCol;

layout(push_constant) uniform Push{
    float random;
    mat4 model;
}push;

void main() {
    // gl_Position = vec4(vPos, 1.f);
    gl_Position = push.model * vec4(vPos, 1.f);
    fCol = (sin(push.random * (1+gl_VertexIndex)) * 0.5f + 0.5f) * vCol;
}