#include "simpleRenderSubSystem.hpp"

#include "buffer.hpp"
#include "componentModel.hpp"
#include "componentTransform.hpp"
#include "descriptors.hpp"
#include "ecsSimpleRenderSystem.hpp"
#include "swapChain.hpp"
#include "vulkanApp.hpp"
#include "gameCoordinator.hpp"
#include "components.hpp"

#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace BE{

void SimpleRenderSubSystem::renderGameObjects(FrameInfo& frameInfo){
    uint32_t frameIndex = frameInfo._FrameIndex;

    // update UBOs
    CameraUbo cameraUbo{};
    cameraUbo._Proj = frameInfo._Camera->getPerspective();
    cameraUbo._View = frameInfo._Camera->getView();
    _CameraUBO[frameIndex]->writeToBuffer(&cameraUbo);

    LightUbo lightUbo{};
    _LightUBO[frameIndex]->writeToBuffer(&lightUbo);

    frameInfo._GlobalDescriptorSet = _GlobalDescriptorSets[frameIndex];
    frameInfo._LightDescriptorSet = _LightDescriptorSets[frameIndex];

    _ECSRenderSystem->renderGameObjects(
        frameInfo, 
        _Pipeline, 
        _PipelineLayout
    );
}

void SimpleRenderSubSystem::initPipelineLayout(){
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
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        _GlobalSetLayout->getDescriptorSetLayout(),
        _LightSetLayout->getDescriptorSetLayout()
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

void SimpleRenderSubSystem::initPipeline(VkRenderPass renderPass){
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
    _Pipeline->initVertexShader("shaders/compiled/basicTriangleVert.spv");
    _Pipeline->initFragmentShader("shaders/compiled/basicTriangleFrag.spv");
    auto pipelineConfig = Pipeline::defaultPipelineConfigInfo();
    pipelineConfig._RenderPass = renderPass;
    pipelineConfig._PipelineLayout = _PipelineLayout;
    _Pipeline->init(pipelineConfig);
}

void SimpleRenderSubSystem::cleanUpPipelineLayout(){
    vkDestroyPipelineLayout(
        _VulkanApp->getDevice(), 
        _PipelineLayout, 
        nullptr
    );
}

void SimpleRenderSubSystem::initECSRender(){
    _ECSRenderSystem = GameCoordinator::registerSystem<ECSSimpleRenderSystem>();
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
    GameCoordinator::setSystemSignature<ECSSimpleRenderSystem>(signature);
}

void SimpleRenderSubSystem::initUBOs(){
    for(int i=0; i<int(_CameraUBO.size()); i++){
        _CameraUBO[i] = BufferPtr(new Buffer(
            _VulkanApp, 
            sizeof(CameraUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _VulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        ));

        _CameraUBO[i]->map();

        _LightUBO[i] = BufferPtr(new Buffer(
            _VulkanApp, 
            sizeof(LightUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _VulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        ));

        _LightUBO[i]->map();
    }
}

void SimpleRenderSubSystem::initDescriptors(){
    _GlobalSetLayout = DescriptorSetLayoutPtr( 
        DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build()
    );

    _LightSetLayout = DescriptorSetLayoutPtr( 
        DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build()
    );

    for(int i=0; i < SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT; i++){
        auto cameraBufferInfo = _CameraUBO[i]->descriptorInfo();
        auto lightBufferInfo = _LightUBO[i]->descriptorInfo();
        
        DescriptorWriter(*_GlobalSetLayout, *_GlobalPool)
            .writeBuffer(0, &cameraBufferInfo)
            .build(_GlobalDescriptorSets[i]);
        DescriptorWriter(*_LightSetLayout, *_GlobalPool)
            .writeBuffer(0, &lightBufferInfo)
            .build(_LightDescriptorSets[i]);
    }
}

};