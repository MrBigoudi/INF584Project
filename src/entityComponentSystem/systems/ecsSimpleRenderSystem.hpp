#pragma once

#include "gameSystem.hpp"
#include "pipeline.hpp"

#include <vulkan/vulkan.hpp>
#include <memory>

class ECSSimpleRenderSystem;
using ECSSimpleRenderSystemPtr = std::shared_ptr<ECSSimpleRenderSystem>;

class ECSSimpleRenderSystem : public GameSystem{

    public:
        ECSSimpleRenderSystem(){}

    public:
        void renderGameObjects(
            VkCommandBuffer commandBufferm, 
            PipelinePtr pipeline,
            VkPipelineLayout pipelineLayout
        );

        void cleanUpGameObjects();

};