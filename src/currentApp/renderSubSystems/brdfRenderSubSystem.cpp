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

    for(int i=0; i<int(_CameraUBO.size()); i++){
        if(_CameraUBO[i])
            _CameraUBO[i]->cleanUp();
        if(_LightUBO[i])
            _LightUBO[i]->cleanUp();
        if(_MaterialUBO[i])
            _MaterialUBO[i]->cleanUp();
    }

}


void BrdfRenderSubSystem::renderingFunction(be::GameObject object){
    be::ModelPtr model = be::GameCoordinator::getComponent<be::ComponentModel>(object)._Model;

    SimplePushConstantData push{};
    auto objectTransform = be::GameCoordinator::getComponent<be::ComponentTransform>(object);
    push._Model = objectTransform._Transform->getModel();
    auto viewModel = _FrameInfo._Camera->getView() * push._Model;
    push._NormalMat = be::Matrix4x4::transpose(be::Matrix4x4::inverse(viewModel));
    
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
    CameraUbo cameraUbo{};
    cameraUbo._Proj = frameInfo._Camera->getPerspective();
    cameraUbo._View = frameInfo._Camera->getView();
    _CameraUBO[frameIndex]->writeToBuffer(&cameraUbo);

    LightUbo lightUbo{};

    lightUbo._NbPointLights = 1;
    lightUbo._PointLights[0] = {
        ._Position = {-3.f, 0.f, 0.f},
        ._Color = {1.f, 0.f, 0.f},
        ._Intensity = 1.f
    };
    for(int i=1; i<MAX_NB_POINT_LIGHTS; i++){
        be::Vector3 curPosition = {
            (std::rand() % 1000) / 50.f - 5.f,
            (std::rand() % 1000) / 50.f - 5.f, 
            (std::rand() % 1000) / 50.f - 5.f
        };
        be::Vector3 curColor = {
            (std::rand() % 256) / 255.f,
            (std::rand() % 256) / 255.f, 
            (std::rand() % 256) / 255.f
        };
        lightUbo._PointLights[i] = {
            ._Position = curPosition,
            ._Color = curColor,
            ._Intensity = 1.f
        };
        lightUbo._NbPointLights++;
    }
    
    // lightUbo._NbDirectionalLights = 1;
    // lightUbo._DirectionalLights[0] = {
    //     ._Direction = {-1.f, -1.f, 0.f},
    //     ._Color = {0.f, 1.f, 1.f},
    //     ._Intensity = 1.f
    // };
    _LightUBO[frameIndex]->writeToBuffer(&lightUbo);

    MaterialUbo materialUbo{};
    auto objectMaterial = be::GameCoordinator::getComponent<be::ComponentMaterial>(object);
    materialUbo._ObjMaterial = objectMaterial;
    _MaterialUBO[frameIndex]->writeToBuffer(&objectMaterial);

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
    be::ErrorHandler::vulkanError(result, "Failed to create pipeline layout!\n");
}

void BrdfRenderSubSystem::initPipeline(VkRenderPass renderPass){
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

    // init pipelines
    for(uint32_t i=0; i<_NB_PIPELINES; i++){
        _PossiblePipelines[i] = be::PipelinePtr(new be::Pipeline(_VulkanApp));
        if(i==0) _PossiblePipelines[i]->initDisneyShaders();// 0 = disney brdf
        if(i==1) _PossiblePipelines[i]->initNormalPassThroughShaders();// 1 = normals brdf
        if(i==2) _PossiblePipelines[i]->initLambertShaders();// 2 = lambert brdf
        if(i==3) _PossiblePipelines[i]->initBlinnPhongShaders();// 3 = blinn-phong brdf
        if(i==4) _PossiblePipelines[i]->initColorPassThroughShaders();// 4 = colors brdf
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
        pipelineConfig._PipelineLayout = _PipelineLayout;
        _WireframePipelines[i]->init(pipelineConfig);
    }

    _Pipeline = _PossiblePipelines[0];
}

void BrdfRenderSubSystem::cleanUpPipelineLayout(){
    vkDestroyPipelineLayout(
        _VulkanApp->getDevice(), 
        _PipelineLayout, 
        nullptr
    );
}


void BrdfRenderSubSystem::initUBOs(){
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

        _LightUBO[i] = be::BufferPtr(new be::Buffer(
            _VulkanApp, 
            sizeof(LightUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _VulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        ));
        _LightUBO[i]->map();

        _MaterialUBO[i] = be::BufferPtr(new be::Buffer(
            _VulkanApp, 
            sizeof(MaterialUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _VulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        ));
        _MaterialUBO[i]->map();
    }
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
        auto cameraBufferInfo = _CameraUBO[i]->descriptorInfo();
        auto lightBufferInfo = _LightUBO[i]->descriptorInfo();
        auto materialBufferInfo = _MaterialUBO[i]->descriptorInfo();
        
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