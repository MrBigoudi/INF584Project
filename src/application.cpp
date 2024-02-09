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

    GameObject triangle = GameCoordinator::createObject();

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
void Application::initRenderSubSystem(){
    if(_Renderer == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a render subsystem without a renderer!\n"
        );
    }
    _RenderSubSystem = SimpleRenderSubSystemPtr(
                        new SimpleRenderSubSystem(
                            _VulkanApp, 
                            _Renderer->getSwapChainRenderPass()
                            )
                        );
}


// CLEANUP FUNCTIONS
void Application::cleanUpWindow(){
    _Window->cleanUp();
}
void Application::cleanUpVulkan(){
    _VulkanApp->cleanUp();
}
void Application::cleanUpRenderer(){
    _Renderer->cleanUp();
}
void Application::cleanUpRenderSubSystem(){
    _RenderSubSystem->cleanUp();
}


// OTHER FUNCTIONS
void Application::mainLoop(){
    while(!_Window->shouldClose()){
        glfwPollEvents();

        auto commandBuffer = _Renderer->beginFrame();
        if(commandBuffer){
            _Renderer->beginSwapChainRenderPass(commandBuffer);
            _RenderSubSystem->renderGameObjects(commandBuffer);
            _Renderer->endSwapChainRenderPass(commandBuffer);
            _Renderer->endFrame();
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}