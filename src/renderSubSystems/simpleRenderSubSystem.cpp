#include "simpleRenderSubSystem.hpp"

#include "vulkanApp.hpp"
#include "entity.hpp"
#include "gameCoordinator.hpp"


void SimpleRenderSubSystem::renderGameObjects(VkCommandBuffer commandBuffer){
    _ECSRenderSystem->renderGameObjects(
        commandBuffer, 
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
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
    _ECSRenderSystem->setCamera(_Camera);

    GameCoordinator::registerComponent<EntityModel>();
    GameCoordinator::registerComponent<EntityTransform>();

    GameObjectSignature signature;
    signature.set(GameCoordinator::getComponentType<EntityModel>());
    signature.set(GameCoordinator::getComponentType<EntityTransform>());
    GameCoordinator::setSystemSignature<ECSSimpleRenderSystem>(signature);
}