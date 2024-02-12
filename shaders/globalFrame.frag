#version 450

layout(location = 0) in vec4 fCol;

layout(location = 0) out vec4 outColor;

void main() {
    // outColor = fCol;
    outColor = vec4(1.f, 0.f, 0.f, 1.f);
}