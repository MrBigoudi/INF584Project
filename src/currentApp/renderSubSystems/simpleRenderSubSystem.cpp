#include "simpleRenderSubSystem.hpp"

#include <cstdint>


SimpleRenderSubSystem::SimpleRenderSubSystem(be::VulkanAppPtr vulkanApp, VkRenderPass renderPass, be::DescriptorPoolPtr globalPool)
    : IRenderSubSystem(vulkanApp, renderPass), _GlobalPool(globalPool){
    initUBOs();
    initDescriptors();
    initPipelineLayout();
    initPipeline(renderPass);
    initRenderSystem();   
}


void SimpleRenderSubSystem::cleanUp() {
    IRenderSubSystem::cleanUp();
    std::function<void(be::GameObject)> func = 
        std::bind(&SimpleRenderSubSystem::cleanUpFunction, 
        this, 
        std::placeholders::_1
    );
    _RenderSystem->cleanUpGameObjects(
        func
    );

    _GlobalSetLayout->cleanUp();
    // _LightSetLayout->cleanUp();

    for(int i=0; i<int(_CameraUBO.size()); i++){
        if(_CameraUBO[i])
            _CameraUBO[i]->cleanUp();
        // if(_LightUBO[i])
        //     _LightUBO[i]->cleanUp();
    }

}

void SimpleRenderSubSystem::cleanUpFunction(be::GameObject object){
    be::ModelPtr model = be::GameCoordinator::getComponent<be::ComponentModel>(object)._Model;
    model->cleanUp();
    be::GameCoordinator::destroyObject(object);
}


void SimpleRenderSubSystem::renderingFunction(be::GameObject object){
    auto rss = be::GameCoordinator::getComponent<be::ComponentRenderSubSystem>(object);
    if(rss._RenderSubSystem->getPipeline() != _Pipeline)
        return;

    be::ModelPtr model = be::GameCoordinator::getComponent<be::ComponentModel>(object)._Model;

    SimplePushConstantData push{};
    // push._Random = static_cast<float>(glfwGetTime());
    auto objectTransform = be::GameCoordinator::getComponent<be::ComponentTransform>(object);
    push._Model = objectTransform.getModel();
    
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

void SimpleRenderSubSystem::renderGameObjects(be::FrameInfo& frameInfo){
    _FrameInfo = frameInfo;
    uint32_t frameIndex = frameInfo._FrameIndex;

    // update UBOs
    CameraUbo cameraUbo{};
    cameraUbo._Proj = frameInfo._Camera->getPerspective();
    cameraUbo._View = frameInfo._Camera->getView();
    _CameraUBO[frameIndex]->writeToBuffer(&cameraUbo);

    // LightUbo lightUbo{};
    // _LightUBO[frameIndex]->writeToBuffer(&lightUbo);

    auto descriptorSets = {
        _GlobalDescriptorSets[frameIndex],
        // _LightDescriptorSets[frameIndex]
    };

    std::function<void(be::GameObject)> func = 
        std::bind(&SimpleRenderSubSystem::renderingFunction, 
        this, 
        std::placeholders::_1
    );

    _RenderSystem->renderGameObjects(
        frameInfo, 
        _Pipeline, 
        _PipelineLayout,
        descriptorSets,
        func
    );
}

void SimpleRenderSubSystem::initPipelineLayout(){
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
        // _LightSetLayout->getDescriptorSetLayout()
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

void SimpleRenderSubSystem::initPipeline(VkRenderPass renderPass){
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
    // _Pipeline->initVertexShader("shaders/compiled/basicTriangleVert.spv");
    // _Pipeline->initFragmentShader("shaders/compiled/basicTriangleFrag.spv");
    _Pipeline->initColorPassThroughShaders();
    auto pipelineConfig = be::Pipeline::defaultPipelineConfigInfo();
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

void SimpleRenderSubSystem::initRenderSystem(){
    _RenderSystem = be::GameCoordinator::registerSystem<be::RenderSystem>();
    if(_RenderSystem == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR,
            "Failed to initialize the ECS render system!\n"
        );
    }

    be::GameObjectSignature signature;
    signature.set(be::GameCoordinator::getComponentType<be::ComponentModel>());
    signature.set(be::GameCoordinator::getComponentType<be::ComponentTransform>());
    signature.set(be::GameCoordinator::getComponentType<be::ComponentRenderSubSystem>());
    be::GameCoordinator::setSystemSignature<be::RenderSystem>(signature);
}

void SimpleRenderSubSystem::initUBOs(){
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

        // _LightUBO[i] = be::BufferPtr(new be::Buffer(
        //     _VulkanApp, 
        //     sizeof(LightUbo),
        //     1,
        //     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        //     _VulkanApp->getProperties().limits.minUniformBufferOffsetAlignment
        // ));

        // _LightUBO[i]->map();
    }
}

void SimpleRenderSubSystem::initDescriptors(){
    _GlobalSetLayout = be::DescriptorSetLayoutPtr( 
        be::DescriptorSetLayout::Builder(_VulkanApp)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build()
    );

    // _LightSetLayout = be::DescriptorSetLayoutPtr( 
    //     be::DescriptorSetLayout::Builder(_VulkanApp)
    //         .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
    //         .build()
    // );

    for(int i=0; i < be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT; i++){
        auto cameraBufferInfo = _CameraUBO[i]->descriptorInfo();
        // auto lightBufferInfo = _LightUBO[i]->descriptorInfo();
        
        be::DescriptorWriter(*_GlobalSetLayout, *_GlobalPool)
            .writeBuffer(0, &cameraBufferInfo)
            .build(_GlobalDescriptorSets[i]);
        // be::DescriptorWriter(*_LightSetLayout, *_GlobalPool)
        //     .writeBuffer(0, &lightBufferInfo)
        //     .build(_LightDescriptorSets[i]);
    }
}