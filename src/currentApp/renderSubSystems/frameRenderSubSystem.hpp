#pragma once

#include <memory>

#include <BigoudiEngine.hpp>

class FrameRenderSubSystem;
using FrameRenderSubSystemPtr = std::shared_ptr<FrameRenderSubSystem>;

class FrameRenderSubSystem : public be::IRenderSubSystem {
    public:
        static const uint32_t _NB_SETS = 1;

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
        FrameRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool);

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