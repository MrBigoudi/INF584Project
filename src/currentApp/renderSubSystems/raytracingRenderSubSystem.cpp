#include "raytracingRenderSubSystem.hpp"

#include <cstdint>
#include "data.hpp"


RaytracingRenderSubSystem::RaytracingRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool)
    : IRenderSubSystem(vulkanApp, renderPass), _GlobalPool(globalPool){
    _RenderPass = renderPass;
}


void RaytracingRenderSubSystem::cleanUp() {
    if(_IsInit){
        IRenderSubSystem::cleanUp();
        _TextureSetLayout->cleanUp();
        for(auto& image : _TextureImage)
            image.cleanUp();
    }
}


void RaytracingRenderSubSystem::renderingFunction(be::GameObject object){
    be::ModelPtr model = be::GameCoordinator::getComponent<be::ComponentModel>(object)._Model;

    model->bind(_FrameInfo._CommandBuffer);
    model->draw(_FrameInfo._CommandBuffer);
}

void RaytracingRenderSubSystem::renderGameObjects(be::FrameInfo& frameInfo){    
    _FrameInfo = frameInfo;
    uint32_t frameIndex = frameInfo._FrameIndex;

    _DescriptorSets = {
        _TextureDescriptorSets[frameIndex],
    };

    be::RenderSystem::renderGameObjects(
        frameInfo, 
        this
    );
}

void RaytracingRenderSubSystem::initPipelineLayout(){
    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(__FILE__, __LINE__, 
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
        _TextureSetLayout->getDescriptorSetLayout(),
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
    be::ErrorHandler::vulkanError(__FILE__, __LINE__, result, "Failed to create pipeline layout!\n");
}

void RaytracingRenderSubSystem::initPipeline(VkRenderPass renderPass){
    if(_Pipeline != nullptr){
        _Pipeline->cleanUp();
    }

    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(__FILE__, __LINE__, 
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a vulkan app!\n"
        );
    }

    if(_PipelineLayout == nullptr){
        be::ErrorHandler::handle(__FILE__, __LINE__, 
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a pipeline layout!\n"
        );
    }

    _Pipeline = be::PipelinePtr(new be::Pipeline(_VulkanApp));
    _Pipeline->initRayTracingShaders();
    auto pipelineConfig = be::Pipeline::defaultPipelineConfigInfo();
    pipelineConfig._RenderPass = renderPass;
    pipelineConfig._RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig._PipelineLayout = _PipelineLayout;
    _Pipeline->init(pipelineConfig);
}

void RaytracingRenderSubSystem::cleanUpPipelineLayout(){
    vkDestroyPipelineLayout(
        _VulkanApp->getDevice(), 
        _PipelineLayout, 
        nullptr
    );
}

void RaytracingRenderSubSystem::updateImage(be::ImagePtr image){
    if(_IsInit){
        cleanUpPipelineLayout();
        _TextureSetLayout->cleanUp();    
        _GlobalPool->resetPool(); // need to free sets
    }
    initImage(image);
    initDescriptors();
    initPipelineLayout();
    initPipeline(_RenderPass);
    _IsInit = true;
}



void RaytracingRenderSubSystem::initImage(be::ImagePtr image){
    for(uint32_t i=0; i<be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT; i++){
        if(_TextureImage[i]._TexChannels != 0){// TODO: change this, != 0 just to check if not null
            _TextureImage[i].cleanUp();
        }
        _TextureImage[i] = be::TextureImage::load(_VulkanApp, image);
        // _TextureImage[i] = be::TextureImage::load(_VulkanApp, "./tmp/ray_tracer.png");
    }
}

void RaytracingRenderSubSystem::initDescriptors(){
    _TextureSetLayout = be::DescriptorSetLayoutPtr( 
        be::DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build()
    );

    for(int i=0; i < be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT; i++){
        auto textureBufferInfo = _TextureImage[i].getDescriptorInfo();
        
        be::DescriptorWriter(*_TextureSetLayout, *_GlobalPool)
            .writeImage(0, &textureBufferInfo)
            .build(_TextureDescriptorSets[i]);
    }
}