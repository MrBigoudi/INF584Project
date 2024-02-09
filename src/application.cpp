#include "application.hpp"

#include "entity.hpp"
#include "errorHandler.hpp"
#include "model.hpp"
#include "gameCoordinator.hpp"
#include "vulkanApp.hpp"

// INIT FUNCTIONS
void Application::initWindow(){
    _Window = WindowPtr(
        new Window(
            WINDOW_WIDTH, 
            WINDOW_HEIGHT, 
            "Hello vulkan!"
        )
    );
    _Window->init();
}
void Application::initVulkan(){
    _VulkanApp = VulkanAppPtr(new VulkanApp());
    _VulkanApp->init(ApplicationPtr(this));
}
void Application::initGameObjects(){
    if(_VulkanApp == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create game objects without a vulkan app!\n"
        );
    }
    std::vector<VertexData> vertices{
        {{0.f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{0.5f, 0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}}
    };

    ModelPtr model = ModelPtr(new Model(_VulkanApp, vertices));

    GameCoordinator::registerComponent<EntityModel>();
    GameCoordinator::registerComponent<EntityTransform>();

    _RenderSystem = GameCoordinator::registerSystem<RenderSystem>();
    if(_RenderSystem == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR,
            "Failed to initialize the ECS render system!\n"
        );
    }

    GameObjectSignature signature;
    signature.set(GameCoordinator::getComponentType<EntityModel>());
    signature.set(GameCoordinator::getComponentType<EntityTransform>());
    GameCoordinator::setSystemSignature<RenderSystem>(signature);

    GameObject triangle = GameCoordinator::createObject();

    _GameObjects.push_back(triangle);
    GameCoordinator::addComponent(
        triangle, 
        EntityModel{
            ._Model = model
        }
    );
    GameCoordinator::addComponent(
        triangle, 
        EntityTransform{}
    );
}
void Application::initPipelineLayout(){
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
void Application::initRenderer(){
    if(_Window == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a renderer without a window!\n"
        );
    }
    if(_VulkanApp == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a renderer without a vulkan app!\n"
        );
    }
    _Renderer = RendererPtr(
            new Renderer(
                _Window, 
                _VulkanApp)
    );
}
void Application::initPipeline(){
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

    if(_Renderer == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a renderer!\n"
        );
    }

    _Pipeline = PipelinePtr(new Pipeline(_VulkanApp));
    _Pipeline->initVertexShader("shaders/compiled/basicTriangleVert.spv");
    _Pipeline->initFragmentShader("shaders/compiled/basicTriangleFrag.spv");
    auto pipelineConfig = Pipeline::defaultPipelineConfigInfo();
    pipelineConfig._RenderPass = _Renderer->getSwapChainRenderPass();
    pipelineConfig._PipelineLayout = _PipelineLayout;
    _Pipeline->init(pipelineConfig);
}


// CLEANUP FUNCTIONS
void Application::cleanUpWindow(){
    _Window->cleanUp();
}
void Application::cleanUpVulkan(){
    _VulkanApp->cleanUp();
}
void Application::cleanUpGameObjects(){
    _RenderSystem->cleanUpGameObjects();
}
void Application::cleanUpPipelineLayout(){   
    vkDestroyPipelineLayout(_VulkanApp->getDevice(), _PipelineLayout, nullptr);
}
void Application::cleanUpRenderer(){
    _Renderer->cleanUp();
}
void Application::cleanUpPipeline(){
    _Pipeline->cleanUp();
}


// OTHER FUNCTIONS
void Application::mainLoop(){
    while(!_Window->shouldClose()){
        glfwPollEvents();

        auto commandBuffer = _Renderer->beginFrame();
        if(commandBuffer){
            _Renderer->beginSwapChainRenderPass(commandBuffer);
            _RenderSystem->renderGameObjects(this, commandBuffer);
            _Renderer->endSwapChainRenderPass(commandBuffer);
            _Renderer->endFrame();
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}