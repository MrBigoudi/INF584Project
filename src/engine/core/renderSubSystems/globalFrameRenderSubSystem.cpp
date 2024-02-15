#include "globalFrameRenderSubSystem.hpp"

#include "buffer.hpp"
#include "descriptors.hpp"
#include "swapChain.hpp"
#include "vulkanApp.hpp"

#include "gameCoordinator.hpp"
#include "components.hpp"

#include <cstdint>

namespace BE{

void GlobalFrameRenderSubSystem::renderGameObjects(FrameInfo& frameInfo){
    uint32_t frameIndex = frameInfo._FrameIndex;

    // update UBOs
    GlobalFrameCameraUbo cameraUbo{};
    cameraUbo._Proj = frameInfo._Camera->getPerspective();
    cameraUbo._View = frameInfo._Camera->getView();
    _CameraUBO[frameIndex]->writeToBuffer(&cameraUbo);

    frameInfo._GlobalDescriptorSet = _GlobalDescriptorSets[frameIndex];

    _ECSRenderSystem->renderGameObjects(
        frameInfo, 
        _Pipeline, 
        _PipelineLayout
    );
}

void GlobalFrameRenderSubSystem::initPipelineLayout(){
    if(_VulkanApp == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline layout without a vulkan app!\n"
        );
    }

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = 
        VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(GlobalFramePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        _GlobalSetLayout->getDescriptorSetLayout()
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    VkResult result = vkCreatePipelineLayout(
        _VulkanApp->getDevice(), 
        &pipelineLayoutInfo, 
        nullptr, 
        &_PipelineLayout
    );
    ErrorHandler::vulkanError(result, "Failed to create pipeline layout!\n");
}

void GlobalFrameRenderSubSystem::initPipeline(VkRenderPass renderPass){
    if(_Pipeline != nullptr){
        _Pipeline->cleanUp();
    }

    if(_VulkanApp == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a vulkan app!\n"
        );
    }

    if(_PipelineLayout == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a pipeline layout!\n"
        );
    }

    _Pipeline = PipelinePtr(new Pipeline(_VulkanApp));
    _Pipeline->initVertexShader("shaders/compiled/globalFrameVert.spv");
    _Pipeline->initFragmentShader("shaders/compiled/globalFrameFrag.spv");
    auto pipelineConfig = Pipeline::defaultPipelineConfigInfo();
    pipelineConfig._RenderPass = renderPass;
    pipelineConfig._PipelineLayout = _PipelineLayout;
    _Pipeline->init(pipelineConfig);
}

void GlobalFrameRenderSubSystem::cleanUpPipelineLayout(){
    vkDestroyPipelineLayout(
        _VulkanApp->getDevice(), 
        _PipelineLayout, 
        nullptr
    );
}

void GlobalFrameRenderSubSystem::initUBOs(){
    for(int i=0; i<int(_CameraUBO.size()); i++){
        _CameraUBO[i] = BufferPtr(new Buffer(
            _VulkanApp, 
            sizeof(GlobalFrameCameraUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _VulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        ));

        _CameraUBO[i]->map();
    }
}

void GlobalFrameRenderSubSystem::initDescriptors(){
    _GlobalSetLayout = DescriptorSetLayoutPtr( 
        DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build()
    );

    for(int i=0; i < SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT; i++){
        auto bufferInfo = _CameraUBO[i]->descriptorInfo();
        DescriptorWriter(*_GlobalSetLayout, *_GlobalPool)
            .writeBuffer(0, &bufferInfo)
            .build(_GlobalDescriptorSets[i]);
    }
}

void GlobalFrameRenderSubSystem::initECSRender(){
    _ECSRenderSystem = GameCoordinator::registerSystem<ECStest>();
    if(_ECSRenderSystem == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR,
            "Failed to initialize the ECS render system!\n"
        );
    }

    GameObjectSignature signature;
    signature.set(GameCoordinator::getComponentType<ComponentModel>());
    signature.set(GameCoordinator::getComponentType<ComponentTransform>());
    signature.set(GameCoordinator::getComponentType<ComponentRenderSubSystem>());
    GameCoordinator::setSystemSignature<ECStest>(signature);
}

};