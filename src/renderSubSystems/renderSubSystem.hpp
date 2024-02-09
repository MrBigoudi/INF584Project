#pragma once

#include "types.hpp"
#include "pipeline.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

class IRenderSubSystem{

    protected:
        VulkanAppPtr _VulkanApp = nullptr;
        PipelinePtr _Pipeline = nullptr;
        VkPipelineLayout _PipelineLayout;

    public:
        IRenderSubSystem(VulkanAppPtr vulkanApp, VkRenderPass renderPass)
            : _VulkanApp(vulkanApp){}

        virtual ~IRenderSubSystem() = default;
        virtual void renderGameObjects(VkCommandBuffer commandBuffer) = 0;

        virtual void cleanUp(){
            cleanUpPipeline();
            cleanUpPipelineLayout();
        }

    public:
        virtual PipelinePtr getPipeline() const {
            return _Pipeline;
        }

        virtual VkPipelineLayout getPipelineLayout() const {
            return _PipelineLayout;
        }

    protected:
        virtual void initPipelineLayout() = 0;
        virtual void initPipeline(VkRenderPass renderPass) = 0;
        virtual void cleanUpPipelineLayout() = 0;
        virtual void cleanUpPipeline(){
            _Pipeline->cleanUp();
        }

};