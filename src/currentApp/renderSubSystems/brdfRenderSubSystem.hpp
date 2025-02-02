#pragma once

#include <memory>

#include <BigoudiEngine.hpp>

class BrdfRenderSubSystem;
using BrdfRenderSubSystemPtr = std::shared_ptr<BrdfRenderSubSystem>;

enum BRDFModel{
    COLOR_BRDF,
    NORMAL_BRDF,
    LAMBERT_BRDF,
    BLINN_PHONG_BRDF,
    MICROFACET_BRDF,
    DISNEY_BRDF,
};

class BrdfRenderSubSystem : public be::IRenderSubSystem {
    public:
        static const uint32_t _NB_SETS = 3;
        static const uint32_t _NB_PIPELINES = 6;
        static const std::array<std::string, _NB_PIPELINES> _PIPELINE_NAMES;

    protected:
        be::CameraUboContainer _CameraUBO{};
        be::LightUboContainer _LightUBO{};
        be::MaterialUboContainer _MaterialUBO{};
        // std::vector<be::BufferPtr> _CameraUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        // std::vector<be::BufferPtr> _LightUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        // std::vector<be::BufferPtr> _MaterialUBO{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};

        be::DescriptorPoolPtr _GlobalPool = nullptr;

        std::vector<VkDescriptorSet> _GlobalDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        be::DescriptorSetLayoutPtr _GlobalSetLayout = nullptr;
        std::vector<VkDescriptorSet> _LightDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        be::DescriptorSetLayoutPtr _LightSetLayout = nullptr;
        std::vector<VkDescriptorSet> _MaterialDescriptorSets{be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        be::DescriptorSetLayoutPtr _MaterialSetLayout = nullptr;

        be::FrameInfo _FrameInfo{};

        std::vector<be::PipelinePtr> _PossiblePipelines = std::vector<be::PipelinePtr>(_NB_PIPELINES);
        std::vector<be::PipelinePtr> _WireframePipelines = std::vector<be::PipelinePtr>(_NB_PIPELINES);

        be::ScenePtr _Scene = nullptr;

        int _PipelineId = DISNEY_BRDF;
        bool _IsSwitchPipelineKeyPressed = false;
        bool _IsWireframePipelineKeyPressed = false;
        bool _IsWireFrameMode = false;

    public:

        void updateSwitchPipelineKey(){
            if(!_IsSwitchPipelineKeyPressed){
                _IsSwitchPipelineKeyPressed = true;
                switchPipeline();
                resetWireframePipelineKey();
                _IsWireFrameMode = false;
            }
        }

        void updateWireframePipelineKey(){
            if(!_IsWireframePipelineKeyPressed){
                _IsWireframePipelineKeyPressed = true;
                switchWireframe();
                return;
            }
        }

        void resetSwitchPipelineKey(){
            _IsSwitchPipelineKeyPressed = false;
        }

        void resetWireframePipelineKey(){
            _IsWireframePipelineKeyPressed = false;
        }


    public:
        BrdfRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool);

        virtual void renderGameObjects(be::FrameInfo& frameInfo) override;

        virtual void cleanUp() override;

        int getBRDFModel() const {return _PipelineId;}
        void setScene(be::ScenePtr scene){_Scene = scene;}


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
            fprintf(stdout, "Pipeline %s\n", _PIPELINE_NAMES[_PipelineId].c_str());
        }

        void switchWireframe(){
            if(!_IsWireFrameMode){
                _Pipeline = _WireframePipelines[_PipelineId];
                _IsWireFrameMode = true;
            }else{
                _Pipeline = _PossiblePipelines[_PipelineId];
                _IsWireFrameMode = false;
            }
        }

        virtual void updateDescriptorSets(be::GameObject object, be::FrameInfo& frameInfo) override;
};