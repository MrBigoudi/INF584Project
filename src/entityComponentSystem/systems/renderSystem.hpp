#pragma once

#include "gameSystem.hpp"
#include "types.hpp"

#include <vulkan/vulkan.hpp>
#include <memory>

class RenderSystem;
using RenderSystemPtr = std::shared_ptr<RenderSystem>;

class RenderSystem : public GameSystem{

    public:
        RenderSystem(){}

    public:
        void renderGameObjects(const Application* app, VkCommandBuffer commandBuffer);
        void cleanUpGameObjects();

};