#pragma once

#include <BigoudiEngine.hpp>

struct SimplePushConstantData : be::PushConstantData{
    alignas(16) be::Matrix4x4 _Model{1.f};
    alignas(16) be::Matrix4x4 _NormalMat{1.f};
};