#include "applicationTest.hpp"

#include <chrono>

// INIT FUNCTIONS
void Application::initWindow(){
    _Window = BE::WindowPtr(
        new BE::Window(
            WINDOW_WIDTH, 
            WINDOW_HEIGHT, 
            "Hello vulkan!"
        )
    );
    _Window->init();
}
void Application::initVulkan(){
    _VulkanApp = BE::VulkanAppPtr(new BE::VulkanApp());
    _VulkanApp->init(_Window);
}
void Application::initGameObjects(){
    if(_VulkanApp == nullptr){
        BE::ErrorHandler::handle(
            BE::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create game objects without a vulkan app!\n"
        );
    }

    BE::ModelPtr loadedModel = BE::ModelPtr(
        new BE::Model(_VulkanApp, "resources/models/dragon.off")
    );

    BE::GameObject model = BE::GameCoordinator::createObject();

    // check components in ecs render system and render sub system
    BE::GameCoordinator::addComponent(
        model, 
        BE::ComponentModel{
            ._Model = loadedModel
        }
    );
    BE::GameCoordinator::addComponent(
        model, 
        BE::ComponentTransform{}
    );
    BE::GameCoordinator::addComponent(
        model, 
        BE::ComponentRenderSubSystem{
            ._RenderSubSystem = _RenderSubSystem
        }
    );


    // // using frame shaders
    // GameObject globalFrame = GameCoordinator::createObject();

    // // check components in ecs render system and render sub system
    // GameCoordinator::addComponent(
    //     globalFrame, 
    //     ComponentModel{
    //         ._Model = loadedModel
    //     }
    // );
    // GameCoordinator::addComponent(
    //     globalFrame, 
    //     ComponentTransform{
    //         ._Position = {-1.f, 0.f, 0.f}
    //     }
    // );
    // GameCoordinator::addComponent(
    //     globalFrame, 
    //     ComponentRenderSubSystem{
    //         ._RenderSubSystem = _GlobalFrameRenderSubSystem
    //     }
    // );

}
void Application::initCamera(){
    _Camera = BE::CameraPtr(new BE::Camera({0.f,0.f,2.f}));
}
void Application::initRenderer(){
    if(_Window == nullptr){
        BE::ErrorHandler::handle(
            BE::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a renderer without a window!\n"
        );
    }
    if(_VulkanApp == nullptr){
        BE::ErrorHandler::handle(
            BE::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a renderer without a vulkan app!\n"
        );
    }
    _Renderer = BE::RendererPtr(
            new BE::Renderer(
                _Window, 
                _VulkanApp)
    );
}
void Application::initRenderSubSystems(){
    if(_Renderer == nullptr){
        BE::ErrorHandler::handle(
            BE::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a render subsystem without a renderer!\n"
        );
    }
    if(_Camera == nullptr){
        BE::ErrorHandler::handle(
            BE::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a render subsystem without a camera!\n"
        );
    }
    _RenderSubSystem = BE::SimpleRenderSubSystemPtr(
                        new BE::SimpleRenderSubSystem(
                            _VulkanApp, 
                            _Renderer->getSwapChainRenderPass(),
                            _GlobalPool
                            )
                        );

    // _GlobalFrameRenderSubSystem = GlobalFrameRenderSubSystemPtr(
    //     new GlobalFrameRenderSubSystem(
    //         _VulkanApp,
    //         _Renderer->getSwapChainRenderPass(),
    //         _GlobalPoolTmp
    //     )
    // );
}
void Application::initDescriptors(){
    _GlobalPool = BE::DescriptorPool::Builder(_VulkanApp)
        .setMaxSets(2*BE::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2*BE::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .build();

    // _GlobalPoolTmp = DescriptorPool::Builder(_VulkanApp)
    //     .setMaxSets(SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
    //     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
    //     .build();
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
void Application::cleanUpRenderSubSystems(){
    _RenderSubSystem->cleanUp();
    // _GlobalFrameRenderSubSystem->cleanUp();
}
void Application::cleanUpDescriptors(){
    _GlobalPool->cleanUp();
    // _GlobalPoolTmp->cleanUp();
}


// OTHER FUNCTIONS
void Application::mainLoop(){

    auto curTime = std::chrono::high_resolution_clock::now();

    while(!_Window->shouldClose()){
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - curTime).count();
        curTime = newTime;

        float aspect = _Renderer->getSwapChainAspectRatio();
        _Camera->setAspectRatio(aspect);
        _Camera->setDt(frameTime);

        KeyboardInput::moveCamera(_Window, _Camera);
        KeyboardInput::updateMouseMode(_Window);

        auto commandBuffer = _Renderer->beginFrame();
        if(commandBuffer){
            BE::FrameInfo currentFrame{};
            currentFrame._FrameIndex = _Renderer->getFrameIndex();
            currentFrame._FrameTime = frameTime;
            currentFrame._CommandBuffer = commandBuffer;
            currentFrame._Camera = _Camera;

            _Renderer->beginSwapChainRenderPass(commandBuffer);

            _RenderSubSystem->renderGameObjects(currentFrame);
            // _GlobalFrameRenderSubSystem->renderGameObjects(currentFrame);

            _Renderer->endSwapChainRenderPass(commandBuffer);
            _Renderer->endFrame();
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}