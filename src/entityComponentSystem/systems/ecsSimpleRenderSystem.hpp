#pragma once

#include "frameInfo.hpp"
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
            FrameInfo frameInfo, 
            PipelinePtr pipeline,
            VkPipelineLayout pipelineLayout
        );

        void cleanUpGameObjects();
};