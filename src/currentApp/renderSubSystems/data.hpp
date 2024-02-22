#pragma once

#include <BigoudiEngine.hpp>

struct SimplePushConstantData : be::PushConstantData{
    alignas(16) be::Matrix4x4 _Model{1.f};
    alignas(16) be::Matrix4x4 _NormalMat{1.f};
};

struct CameraUbo{
    alignas(16) be::Matrix4x4 _View{1.f};
    alignas(16) be::Matrix4x4 _Proj{1.f};
};

// TODO: move this in the engine
struct PointLight{
    alignas(16) be::Vector3 _Position{};
    alignas(16) be::Vector3 _Color{};
    alignas(4) float _Intensity = 1.f;
};

struct DirectionalLight{
    alignas(16) be::Vector3 _Direction{};
    alignas(16) be::Vector3 _Color{};
    alignas(4) float _Intensity = 1.f;
};

const int MAX_NB_POINT_LIGHTS = 10;
const int MAX_NB_DIRECTIONAL_LIGHTS = 10;

struct LightUbo{
    alignas(4) uint32_t _NbPointLights = 0;
    alignas(16) PointLight _PointLights[MAX_NB_POINT_LIGHTS];
    alignas(4) uint32_t _NbDirectionalLights = 0;
    alignas(16) DirectionalLight _DirectionalLights[MAX_NB_DIRECTIONAL_LIGHTS];
};

struct MaterialUbo{
    be::ComponentMaterial _ObjMaterial{};
};