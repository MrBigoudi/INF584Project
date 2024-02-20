#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vCol;
layout(location = 2) in vec3 vNorm;
layout(location = 3) in vec2 vTex;

layout(location = 0) out vec4 fCol;

layout(set = 0, binding = 0) uniform CameraUbo{
    mat4 view;
    mat4 proj;
} cameraUbo;

layout(push_constant) uniform Push{
    mat4 model;
}push;

void main() {
    gl_Position = cameraUbo.proj * cameraUbo.view * push.model * vec4(vPos, 1.f);
    // fCol = vec4(vNorm*0.5f + 0.5f, 1.f);
    fCol = vec4(vNorm, 1.f);
}