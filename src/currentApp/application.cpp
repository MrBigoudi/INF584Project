#include "applicationTest.hpp"

#include <chrono>
#include "keyboardInput.hpp"

// INIT FUNCTIONS
void Application::initWindow(){
    _Window = be::WindowPtr(
        new be::Window(
            WINDOW_WIDTH, 
            WINDOW_HEIGHT, 
            "Hello vulkan!"
        )
    );
    _Window->init();
}
void Application::initVulkan(){
    _VulkanApp = be::VulkanAppPtr(new be::VulkanApp());
    _VulkanApp->init(_Window);
}
void Application::initGameObjects(){
    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create game objects without a vulkan app!\n"
        );
    }

    be::ModelPtr frame = be::ModelPtr(
        new be::Model(
            _VulkanApp, 
            be::VertexDataBuilder::primitiveFrame()
        )
    );

    be::GameObject object = be::RenderSystem::createRenderableObject(
        {._Model = frame}, 
        {},
        {._RenderSubSystem = _RenderSubSystem} 
    );
    _GameObjects.push_back(object);


    // load face model
    be::ModelPtr faceModel = be::ModelPtr(
        // new be::Model(_VulkanApp, "resources/models/dragon.off")
        new be::Model(_VulkanApp, "resources/models/face.off")
    );

    object = be::RenderSystem::createRenderableObject(
        {._Model = faceModel}, 
        {
            ._Position = {2.f, -1.f, 0.f},
            ._Scale = {0.01f, 0.01f, 0.01f},
        },
        {._RenderSubSystem = _NormalRenderSubSystem} 
    );
    _GameObjects.push_back(object);

}
void Application::initCamera(){
    _Camera = be::CameraPtr(new be::Camera(be::Vector3(0.f,0.f,2.f)));
}
void Application::initRenderer(){
    if(_Window == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a renderer without a window!\n"
        );
    }
    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a renderer without a vulkan app!\n"
        );
    }
    _Renderer = be::RendererPtr(
            new be::Renderer(
                _Window, 
                _VulkanApp)
    );
}
void Application::initSystems(){
    be::RenderSystem::init();
}
void Application::initRenderSubSystems(){
    if(_Renderer == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a render subsystem without a renderer!\n"
        );
    }
    if(_Camera == nullptr){
        be::ErrorHandler::handle(
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
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

    _NormalRenderSubSystem = NormalRenderSubSystemPtr(
                        new NormalRenderSubSystem(
                            _VulkanApp, 
                            _Renderer->getSwapChainRenderPass(),
                            _GlobalPool
                            )
                        );
}
void Application::initDescriptors(){
    _GlobalPool = be::DescriptorPool::Builder(_VulkanApp)
        .setMaxSets(2*be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2*be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
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
    _NormalRenderSubSystem->cleanUp();
}
void Application::cleanUpDescriptors(){
    _GlobalPool->cleanUp();
}
void Application::cleanUpGameObjects(){
    for(auto object: _GameObjects){
        be::ModelPtr model = be::GameCoordinator::getComponent<be::ComponentModel>(object)._Model;
        model->cleanUp();
        be::GameCoordinator::destroyObject(object);
    }
    _GameObjects.clear();
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
            be::FrameInfo currentFrame{};
            currentFrame._FrameIndex = _Renderer->getFrameIndex();
            currentFrame._FrameTime = frameTime;
            currentFrame._CommandBuffer = commandBuffer;
            currentFrame._Camera = _Camera;

            _Renderer->beginSwapChainRenderPass(commandBuffer);

            _RenderSubSystem->renderGameObjects(currentFrame);
            _NormalRenderSubSystem->renderGameObjects(currentFrame);

            _Renderer->endSwapChainRenderPass(commandBuffer);
            _Renderer->endFrame();
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}