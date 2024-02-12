#pragma once

#include "renderSubSystem.hpp"
#include "types.hpp"
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

#include "ecsSimpleRenderSystem.hpp"
#include "buffer.hpp"
#include "swapChain.hpp"

#include "descriptors.hpp"

struct SimplePushConstantData{
    alignas(4) float _Random;
    alignas(16) glm::mat4 _Model;
};

struct CameraUbo{
    glm::mat4 _View{1.f};
    glm::mat4 _Proj{1.f};
};

struct LightUbo{
    glm::vec3 _LightDir = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
};

class SimpleRenderSubSystem;
using SimpleRenderSubSystemPtr = std::shared_ptr<SimpleRenderSubSystem>;

class SimpleRenderSubSystem : public IRenderSubSystem{
    private:
        ECSSimpleRenderSystemPtr _ECSRenderSystem = nullptr;

        std::vector<BufferPtr> _CameraUBO{SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        std::vector<BufferPtr> _LightUBO{SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};

        DescriptorPoolPtr _GlobalPool = nullptr;
        std::vector<VkDescriptorSet> _GlobalDescriptorSets{SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT};
        DescriptorSetLayoutPtr _GlobalSetLayout = nullptr;

    public:
        SimpleRenderSubSystem(VulkanAppPtr vulkanApp, VkRenderPass renderPass, DescriptorPoolPtr globalPool)
            : IRenderSubSystem(vulkanApp, renderPass), _GlobalPool(globalPool){
            initUBOs();
            initDescriptors();
            initPipelineLayout();
            initPipeline(renderPass);
            initECSRender();   
        }

        void renderGameObjects(FrameInfo& frameInfo) override;

        void cleanUp() override{
            IRenderSubSystem::cleanUp();
            _ECSRenderSystem->cleanUpGameObjects();

            _GlobalSetLayout->cleanUp();

            for(int i=0; i<_CameraUBO.size(); i++){
                if(_CameraUBO[i])
                    _CameraUBO[i]->cleanUp();
                if(_LightUBO[i])
                    _LightUBO[i]->cleanUp();
            }

        }


    private:
        void initECSRender();
        void initPipelineLayout() override;
        void initPipeline(VkRenderPass renderPass) override;
        void cleanUpPipelineLayout() override;
        void initUBOs();
        void initDescriptors();
};