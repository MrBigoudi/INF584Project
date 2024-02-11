#include "application.hpp"

#include "camera.hpp"
#include "entity.hpp"
#include "errorHandler.hpp"
#include "gameObject.hpp"
#include "keyboardInput.hpp"
#include "model.hpp"
#include "gameCoordinator.hpp"
#include "vulkanApp.hpp"

#include <chrono>

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
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},

    };

    ModelPtr model = ModelPtr(new Model(_VulkanApp, vertices));

    GameObject cube = GameCoordinator::createObject();

    // check components in ecs render system and render sub system
    GameCoordinator::addComponent(
        cube, 
        EntityModel{
            ._Model = model
        }
    );
    GameCoordinator::addComponent(
        cube, 
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
                            _Camera
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
            _Renderer->beginSwapChainRenderPass(commandBuffer);
            _RenderSubSystem->renderGameObjects(commandBuffer);
            _Renderer->endSwapChainRenderPass(commandBuffer);
            _Renderer->endFrame();
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}