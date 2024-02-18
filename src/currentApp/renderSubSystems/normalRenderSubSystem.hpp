#pragma once

#include <memory>

#include <BigoudiEngine.hpp>

class NormalRenderSubSystem;
using NormalRenderSubSystemPtr = std::shared_ptr<NormalRenderSubSystem>;

class NormalRenderSubSystem : public be::IRenderSubSystem {
    protected:

        std::vector<be::BufferPtr> _CameraUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};

        be::DescriptorPoolPtr _GlobalPool = nullptr;

        std::vector<VkDescriptorSet> _GlobalDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        be::DescriptorSetLayoutPtr _GlobalSetLayout = nullptr;

        be::FrameInfo _FrameInfo{};

        std::vector<be::PipelinePtr> _PossiblePipelines{};

        int _PipelineId = 0;
        bool _IsSwitchPipelineKeyPressed = false;

    public:

        void updatePipelineKey(){
            if(!_IsSwitchPipelineKeyPressed){
                _IsSwitchPipelineKeyPressed = true;
                switchPipeline();
            }
        }

        void resetPipelineKey(){
            _IsSwitchPipelineKeyPressed = false;
        }


    public:
        NormalRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool);

        virtual void renderGameObjects(be::FrameInfo& frameInfo) override;

        virtual void cleanUp() override;


    protected:
        virtual void initPipelineLayout() override;
        virtual void initPipeline(VkRenderPass renderPass) override;
        virtual void cleanUpPipelineLayout() override;
        virtual void cleanUpPipeline() override;
        virtual void initUBOs();
        virtual void initDescriptors();

        virtual void renderingFunction(be::GameObject object) override;
        void switchPipeline(){
            _PipelineId = (_PipelineId + 1) % _PossiblePipelines.size();
            _Pipeline = _PossiblePipelines[_PipelineId];
        }
};