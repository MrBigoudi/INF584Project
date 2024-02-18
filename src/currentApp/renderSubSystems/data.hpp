#pragma once

#include <BigoudiEngine.hpp>

struct SimplePushConstantData : be::PushConstantData{
    // alignas(4) float _Random;
    alignas(16) be::Matrix4x4 _Model;
};

struct CameraUbo{
    be::Matrix4x4 _View{1.f};
    be::Matrix4x4 _Proj{1.f};
};

struct LightUbo{
    be::Vector4 _LightDir = be::Vector4(
        be::Vector3::normalize(
            be::Vector3(1.f, -3.f, -1.f)
        ), 
        1.f
    );
};