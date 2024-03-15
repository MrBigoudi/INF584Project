#include "brdfRenderSubSystem.hpp"

#include <cstdint>
#include "data.hpp"


BrdfRenderSubSystem::BrdfRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool)
    : IRenderSubSystem(vulkanApp, renderPass), _GlobalPool(globalPool){
    _NeedPerObjectBind = true;
    initUBOs();
    initDescriptors();
    initPipelineLayout();
    initPipeline(renderPass);
}

void BrdfRenderSubSystem::cleanUpPipeline(){
    for(auto& pipeline : _PossiblePipelines){
        pipeline->cleanUp();
    }
    for(auto& pipeline : _WireframePipelines){
        pipeline->cleanUp();
    }
    _PossiblePipelines.clear();
    _WireframePipelines.clear();
}


void BrdfRenderSubSystem::cleanUp() {
    IRenderSubSystem::cleanUp();

    _GlobalSetLayout->cleanUp();
    _LightSetLayout->cleanUp();
    _MaterialSetLayout->cleanUp();

    _CameraUBO.cleanUp();
    _LightUBO.cleanUp();
    _MaterialUBO.cleanUp();
}


void BrdfRenderSubSystem::renderingFunction(be::GameObject object){
    // fprintf(stdout, "obj: %d\n", object);
    be::ModelPtr model = be::GameCoordinator::getComponent<be::ComponentModel>(object)._Model;

    SimplePushConstantData push{};
    auto objectTransform = be::GameCoordinator::getComponent<be::ComponentTransform>(object);
    push._Model = objectTransform._Transform->getModel();
    auto objectMaterial = be::GameCoordinator::getComponent<be::ComponentMaterial>(object);
    push._MaterialId = objectMaterial.getId();
    
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

void BrdfRenderSubSystem::updateDescriptorSets(be::GameObject object, be::FrameInfo& frameInfo){
    uint32_t frameIndex = frameInfo._FrameIndex;
    
    // update UBOs
    _CameraUBO.setProj(frameInfo._Camera->getPerspective());
    _CameraUBO.setView(frameInfo._Camera->getView());
    _CameraUBO.update(frameIndex);

    _LightUBO.reset();
    for(auto light : _Scene->getPointLights()){
        _LightUBO.addPointLight(light);
    }
    for(auto light : _Scene->getDirectionalLights()){
        _LightUBO.addDirectionalLight(light);
    }
    _LightUBO.update(frameIndex);

    auto objectMaterial = be::GameCoordinator::getComponent<be::ComponentMaterial>(object);
    _MaterialUBO.setMaterial(objectMaterial._Material, objectMaterial.getId());
    _MaterialUBO.update(frameIndex);

    _DescriptorSets = {
        _GlobalDescriptorSets[frameIndex],
        _LightDescriptorSets[frameIndex],
        _MaterialDescriptorSets[frameIndex],
    };
}

void BrdfRenderSubSystem::renderGameObjects(be::FrameInfo& frameInfo){    
    _FrameInfo = frameInfo;

    be::RenderSystem::renderGameObjects(
        frameInfo, 
        this
    );
}

void BrdfRenderSubSystem::initPipelineLayout(){
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
        _GlobalSetLayout->getDescriptorSetLayout(),
        _LightSetLayout->getDescriptorSetLayout(),
        _MaterialSetLayout->getDescriptorSetLayout(),
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

void BrdfRenderSubSystem::initPipeline(VkRenderPass renderPass){
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

    // init pipelines
    for(uint32_t i=0; i<_NB_PIPELINES; i++){
        _PossiblePipelines[i] = be::PipelinePtr(new be::Pipeline(_VulkanApp));
        if(i==COLOR_BRDF) _PossiblePipelines[i]->initColorPassThroughShaders();// 0 = colors brdf
        if(i==NORMAL_BRDF) _PossiblePipelines[i]->initNormalPassThroughShaders();// 1 = normals brdf
        if(i==LAMBERT_BRDF) _PossiblePipelines[i]->initLambertShaders();// 2 = lambert brdf
        if(i==BLINN_PHONG_BRDF) _PossiblePipelines[i]->initBlinnPhongShaders();// 3 = blinn-phong brdf
        if(i==MICROFACET_BRDF) _PossiblePipelines[i]->initDisneyShaders();// 4 = disney brdf
        auto pipelineConfig = be::Pipeline::defaultPipelineConfigInfo();
        pipelineConfig._RenderPass = renderPass;
        pipelineConfig._PipelineLayout = _PipelineLayout;
        _PossiblePipelines[i]->init(pipelineConfig);
    }

    // init wireframe pipelines
    for(uint32_t i=0; i<_NB_PIPELINES; i++){
        _WireframePipelines[i] = be::PipelinePtr(new be::Pipeline(_VulkanApp));
        _WireframePipelines[i]->copyShaders(_PossiblePipelines[i]);
        auto pipelineConfig = be::Pipeline::defaultPipelineConfigInfo();
        pipelineConfig = be::Pipeline::defaultWireFramePipelineConfigInfo();
        pipelineConfig._RenderPass = renderPass;
        pipelineConfig._RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig._PipelineLayout = _PipelineLayout;
        _WireframePipelines[i]->init(pipelineConfig);
    }

    _Pipeline = _PossiblePipelines[LAMBERT_BRDF];
}

void BrdfRenderSubSystem::cleanUpPipelineLayout(){
    vkDestroyPipelineLayout(
        _VulkanApp->getDevice(), 
        _PipelineLayout, 
        nullptr
    );
}


void BrdfRenderSubSystem::initUBOs(){
    _CameraUBO.init(be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT, _VulkanApp);
    _LightUBO.init(be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT, _VulkanApp);
    _MaterialUBO.init(be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT, _VulkanApp);
}

void BrdfRenderSubSystem::initDescriptors(){
    _GlobalSetLayout = be::DescriptorSetLayoutPtr( 
        be::DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, 
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
            )
            .build()
    );

    _LightSetLayout = be::DescriptorSetLayoutPtr( 
        be::DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, 
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
            VK_SHADER_STAGE_FRAGMENT_BIT
            )
            .build()
    );

    _MaterialSetLayout = be::DescriptorSetLayoutPtr( 
        be::DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, 
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
            VK_SHADER_STAGE_FRAGMENT_BIT
            )
            .build()
    );

    for(int i=0; i < be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT; i++){
        auto cameraBufferInfo = _CameraUBO.getDescriptorInfo(i);
        auto lightBufferInfo = _LightUBO.getDescriptorInfo(i);
        auto materialBufferInfo = _MaterialUBO.getDescriptorInfo(i);
        
        be::DescriptorWriter(*_GlobalSetLayout, *_GlobalPool)
            .writeBuffer(0, &cameraBufferInfo)
            .build(_GlobalDescriptorSets[i]);
        be::DescriptorWriter(*_LightSetLayout, *_GlobalPool)
            .writeBuffer(0, &lightBufferInfo)
            .build(_LightDescriptorSets[i]);
        be::DescriptorWriter(*_MaterialSetLayout, *_GlobalPool)
            .writeBuffer(0, &materialBufferInfo)
            .build(_MaterialDescriptorSets[i]);
    }
}