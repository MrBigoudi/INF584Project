#pragma once

#include <memory>

#include <BigoudiEngine.hpp>

class NormalRenderSubSystem;
using NormalRenderSubSystemPtr = std::shared_ptr<NormalRenderSubSystem>;

class NormalRenderSubSystem : public be::IRenderSubSystem {
    protected:

        std::vector<be::BufferPtr> _CameraUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        // std::vector<be::BufferPtr> _LightUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};

        be::DescriptorPoolPtr _GlobalPool = nullptr;

        std::vector<VkDescriptorSet> _GlobalDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        // std::vector<VkDescriptorSet> _LightDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        be::DescriptorSetLayoutPtr _GlobalSetLayout = nullptr;
        // be::DescriptorSetLayoutPtr _LightSetLayout = nullptr;

        be::FrameInfo _FrameInfo{};

    public:
        NormalRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool);

        virtual void renderGameObjects(be::FrameInfo& frameInfo) override;

        virtual void cleanUp() override;


    protected:
        virtual void initPipelineLayout() override;
        virtual void initPipeline(VkRenderPass renderPass) override;
        virtual void cleanUpPipelineLayout() override;
        virtual void initUBOs();
        virtual void initDescriptors();

        virtual void renderingFunction(be::GameObject object) override;
};