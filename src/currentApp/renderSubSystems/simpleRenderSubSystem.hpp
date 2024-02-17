#pragma once

#include <memory>

#include <BigoudiEngine.hpp>

struct SimplePushConstantData : be::PushConstantData{
    // alignas(4) float _Random;
    alignas(16) be::Matrix4x4 _Model;
};

struct CameraUbo{
    be::Matrix4x4 _View{1.f};
    be::Matrix4x4 _Proj{1.f};
};

struct LightUbo{
    be::Vector4 _LightDir = be::Vector4(
        be::Vector3::normalize(
            be::Vector3(1.f, -3.f, -1.f)
        ), 
        1.f
    );
};

class SimpleRenderSubSystem;
using SimpleRenderSubSystemPtr = std::shared_ptr<SimpleRenderSubSystem>;

class SimpleRenderSubSystem : public be::IRenderSubSystem {
    private:

        std::vector<be::BufferPtr> _CameraUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        // std::vector<be::BufferPtr> _LightUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};

        be::DescriptorPoolPtr _GlobalPool = nullptr;

        std::vector<VkDescriptorSet> _GlobalDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        // std::vector<VkDescriptorSet> _LightDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        be::DescriptorSetLayoutPtr _GlobalSetLayout = nullptr;
        // be::DescriptorSetLayoutPtr _LightSetLayout = nullptr;

        be::FrameInfo _FrameInfo{};

    public:
        SimpleRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool);

        void renderGameObjects(be::FrameInfo& frameInfo) override;

        void cleanUp() override;


    private:
        void initRenderSystem();
        void initPipelineLayout() override;
        void initPipeline(VkRenderPass renderPass) override;
        void cleanUpPipelineLayout() override;
        void initUBOs();
        void initDescriptors();

        void renderingFunction(be::GameObject object) override;
        void cleanUpFunction(be::GameObject object) override;
};