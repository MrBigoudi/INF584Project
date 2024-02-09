#pragma once

#include "renderSubSystem.hpp"
#include "types.hpp"
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

struct SimplePushConstantData{
    alignas(4) float _Random;
    alignas(16) glm::mat4 _Model;
};

#include "ecsSimpleRenderSystem.hpp"

class SimpleRenderSubSystem;
using SimpleRenderSubSystemPtr = std::shared_ptr<SimpleRenderSubSystem>;

class SimpleRenderSubSystem : public IRenderSubSystem{
    private:
        ECSSimpleRenderSystemPtr _ECSRenderSystem = nullptr;

    public:
        SimpleRenderSubSystem(VulkanAppPtr vulkanApp, VkRenderPass renderPass)
            : IRenderSubSystem(vulkanApp, renderPass){
            initPipelineLayout();
            initPipeline(renderPass);
            initECSRender();
        }

        void renderGameObjects(VkCommandBuffer commandBuffer) override;

        void cleanUp() override{
            IRenderSubSystem::cleanUp();
            _ECSRenderSystem->cleanUpGameObjects();
        }


    private:
        void initECSRender();
        void initPipelineLayout() override;
        void initPipeline(VkRenderPass renderPass) override;
        void cleanUpPipelineLayout() override;
};