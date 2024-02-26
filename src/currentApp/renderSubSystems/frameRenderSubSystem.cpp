#include "frameRenderSubSystem.hpp"

#include <cstdint>
#include "data.hpp"


FrameRenderSubSystem::FrameRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool)
    : IRenderSubSystem(vulkanApp, renderPass), _GlobalPool(globalPool){
    initUBOs();
    initDescriptors();
    initPipelineLayout();
    initPipeline(renderPass);
}


void FrameRenderSubSystem::cleanUp() {
    IRenderSubSystem::cleanUp();

    _GlobalSetLayout->cleanUp();

    for(int i=0; i<int(_CameraUBO.size()); i++){
        if(_CameraUBO[i])
            _CameraUBO[i]->cleanUp();
    }

}


void FrameRenderSubSystem::renderingFunction(be::GameObject object){
    be::ModelPtr model = be::GameCoordinator::getComponent<be::ComponentModel>(object)._Model;

    SimplePushConstantData push{};
    auto objectTransform = be::GameCoordinator::getComponent<be::ComponentTransform>(object);
    push._Model = objectTransform._Transform->getModel();
    
    vkCmdPushConstants(
        _FrameInfo._CommandBuffer, 
        _PipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, 
        sizeof(SimplePushConstantData), 
        &push
    );

    model->bind(_FrameInfo._CommandBuffer);
    model->draw(_FrameInfo._CommandBuffer);
}

void FrameRenderSubSystem::renderGameObjects(be::FrameInfo& frameInfo){
    _FrameInfo = frameInfo;
    uint32_t frameIndex = frameInfo._FrameIndex;

    // update UBOs
    CameraUbo cameraUbo{};
    cameraUbo._Proj = frameInfo._Camera->getPerspective();
    cameraUbo._View = frameInfo._Camera->getView();
    _CameraUBO[frameIndex]->writeToBuffer(&cameraUbo);

    _DescriptorSets = {
        _GlobalDescriptorSets[frameIndex],
    };

    be::RenderSystem::renderGameObjects(
        frameInfo, 
        this
    );
}

void FrameRenderSubSystem::initPipelineLayout(){
    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
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
    be::ErrorHandler::vulkanError(result, "Failed to create pipeline layout!\n");
}

void FrameRenderSubSystem::initPipeline(VkRenderPass renderPass){
    if(_Pipeline != nullptr){
        _Pipeline->cleanUp();
    }

    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a vulkan app!\n"
        );
    }

    if(_PipelineLayout == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a pipeline layout!\n"
        );
    }

    _Pipeline = be::PipelinePtr(new be::Pipeline(_VulkanApp));
    _Pipeline->initColorPassThroughShaders();
    auto pipelineConfig = be::Pipeline::defaultPipelineConfigInfo();
    pipelineConfig._RenderPass = renderPass;
    pipelineConfig._PipelineLayout = _PipelineLayout;
    _Pipeline->init(pipelineConfig);
}

void FrameRenderSubSystem::cleanUpPipelineLayout(){
    vkDestroyPipelineLayout(
        _VulkanApp->getDevice(), 
        _PipelineLayout, 
        nullptr
    );
}


void FrameRenderSubSystem::initUBOs(){
    for(int i=0; i<int(_CameraUBO.size()); i++){
        _CameraUBO[i] = be::BufferPtr(new be::Buffer(
            _VulkanApp, 
            sizeof(CameraUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _VulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        ));

        _CameraUBO[i]->map();
    }
}

void FrameRenderSubSystem::initDescriptors(){
    _GlobalSetLayout = be::DescriptorSetLayoutPtr( 
        be::DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build()
    );

    for(int i=0; i < be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT; i++){
        auto cameraBufferInfo = _CameraUBO[i]->descriptorInfo();
        
        be::DescriptorWriter(*_GlobalSetLayout, *_GlobalPool)
            .writeBuffer(0, &cameraBufferInfo)
            .build(_GlobalDescriptorSets[i]);
    }
}