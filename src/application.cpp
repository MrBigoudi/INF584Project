#include "application.hpp"

#include "camera.hpp"
#include "descriptors.hpp"
#include "entity.hpp"
#include "errorHandler.hpp"
#include "frameInfo.hpp"
#include "gameObject.hpp"
#include "keyboardInput.hpp"
#include "model.hpp"
#include "gameCoordinator.hpp"
#include "swapChain.hpp"
#include "vulkanApp.hpp"

#include <chrono>
#include <cstdint>
#include <vulkan/vulkan_core.h>

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

    ModelPtr loadedModel = ModelPtr(
        // new Model(_VulkanApp, "resources/models/colored_cube.obj")
        new Model(_VulkanApp, "resources/models/sphere.off")
    );

    GameObject sphere = GameCoordinator::createObject();

    // check components in ecs render system and render sub system
    GameCoordinator::addComponent(
        sphere, 
        EntityModel{
            ._Model = loadedModel
        }
    );
    GameCoordinator::addComponent(
        sphere, 
        EntityTransform{}
    );
}
void Application::initCamera(){
    _Camera = CameraPtr(new Camera({0.f,0.f,2.f}));
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
    if(_Camera == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a render subsystem without a camera!\n"
        );
    }
    _RenderSubSystem = SimpleRenderSubSystemPtr(
                        new SimpleRenderSubSystem(
                            _VulkanApp, 
                            _Renderer->getSwapChainRenderPass(),
                            _GlobalPool
                            )
                        );
}
void Application::initDescriptors(){
    _GlobalPool = DescriptorPool::Builder(_VulkanApp)
        .setMaxSets(SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .build();
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
void Application::cleanUpDescriptors(){
    _GlobalPool->cleanUp();
}


// OTHER FUNCTIONS
void Application::mainLoop(){

    auto curTime = std::chrono::high_resolution_clock::now();

    while(!_Window->shouldClose()){
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - curTime).count();
        curTime = newTime;

        KeyboardInput::moveCamera(_Window, frameTime, _Camera);
        KeyboardInput::updateMouseMode(_Window);

        float aspect = _Renderer->getSwapChainAspectRatio();
        _Camera->setAspectRatio(aspect);

        auto commandBuffer = _Renderer->beginFrame();
        if(commandBuffer){
            FrameInfo currentFrame{};
            currentFrame._FrameIndex = _Renderer->getFrameIndex();
            currentFrame._FrameTime = frameTime;
            currentFrame._CommandBuffer = commandBuffer;
            currentFrame._Camera = _Camera;

            _Renderer->beginSwapChainRenderPass(commandBuffer);
            _RenderSubSystem->renderGameObjects(currentFrame);
            _Renderer->endSwapChainRenderPass(commandBuffer);
            _Renderer->endFrame();
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}