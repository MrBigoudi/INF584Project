#pragma once

#include <memory>

#include <BigoudiEngine.hpp>

class RaytracingRenderSubSystem;
using RaytracingRenderSubSystemPtr = std::shared_ptr<RaytracingRenderSubSystem>;

class RaytracingRenderSubSystem : public be::IRenderSubSystem {
    public:
        static const uint32_t _NB_SETS = 1;

    protected:
        std::vector<be::TextureImage> _TextureImage{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        std::vector<VkDescriptorSet> _TextureDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        be::DescriptorSetLayoutPtr _TextureSetLayout = nullptr;
        
        be::DescriptorPoolPtr _GlobalPool = nullptr;

        be::FrameInfo _FrameInfo{};

        VkRenderPass _RenderPass = nullptr;
        bool _IsInit = false;

    public:
        RaytracingRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool);
        virtual void renderGameObjects(be::FrameInfo& frameInfo) override;
        virtual void cleanUp() override;

        void updateImage(be::ImagePtr image);
        bool isInit() const {return _IsInit;}

        void setRenderPass(VkRenderPass renderPass){
            _RenderPass = renderPass;
        }


    protected:
        virtual void initPipelineLayout() override;
        virtual void initPipeline(VkRenderPass renderPass) override;
        virtual void cleanUpPipelineLayout() override;
        virtual void initImage(be::ImagePtr image);
        virtual void initDescriptors();

        virtual void renderingFunction(be::GameObject object) override;
};