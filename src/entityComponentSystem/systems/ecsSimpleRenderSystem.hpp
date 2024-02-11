#pragma once

#include "camera.hpp"
#include "gameSystem.hpp"
#include "pipeline.hpp"

#include <vulkan/vulkan.hpp>
#include <memory>

class ECSSimpleRenderSystem;
using ECSSimpleRenderSystemPtr = std::shared_ptr<ECSSimpleRenderSystem>;

class ECSSimpleRenderSystem : public GameSystem{

    private:
        CameraPtr _Camera = nullptr;

    public:
        ECSSimpleRenderSystem(){}

    public:
        void renderGameObjects(
            VkCommandBuffer commandBufferm, 
            PipelinePtr pipeline,
            VkPipelineLayout pipelineLayout
        );

        void cleanUpGameObjects();

        void setCamera(CameraPtr camera){_Camera = camera;}
};