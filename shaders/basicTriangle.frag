#version 450

layout(location = 0) in vec4 fCol;
layout(location = 1) in vec3 fNorm;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform LightUbo{
    vec4 lightDir;
} lightUbo;

void main() {
    outColor = fCol * dot(vec4(fNorm, 1.f), lightUbo.lightDir);
}