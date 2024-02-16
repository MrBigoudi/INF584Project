#pragma once

#include "camera.hpp"
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace BE{

struct FrameInfo{
    uint32_t _FrameIndex = 0;
    float _FrameTime = 0.f;
    VkCommandBuffer _CommandBuffer;
    CameraPtr _Camera = nullptr;
    VkDescriptorSet _GlobalDescriptorSet = nullptr;
    VkDescriptorSet _LightDescriptorSet = nullptr;
};

};