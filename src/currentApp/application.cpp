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
        be::ErrorHandler::handle(__FILE__, __LINE__, 
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
        {._RenderSubSystem = _RenderSubSystem},
        {._Model = frame}
    );
    _GameObjects.push_back(object);

    be::ModelPtr viewportRectangleModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            2.f, 
            2.f
            )
        )
    );

    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _RaytracingRenderSubSystem},
        {._Model = viewportRectangleModel}
    );
    _GameObjects.push_back(object);




    // load face model
    be::ModelPtr faceModel = be::ModelPtr(
        // new be::Model(_VulkanApp, "resources/models/dragon.off")
        // new be::Model(_VulkanApp, "resources/models/face.off")
        // new be::Model(_VulkanApp, "resources/models/sponza.obj")
        // new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveSphere())
        // new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveTriangle())
        // new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle())
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveCube())
    );

    be::TransformPtr faceTransform = be::TransformPtr(
        new be::Transform()
    );
    // faceTransform->_Scale = {0.01f, 0.01f, 0.01f};
    faceTransform->_Scale = {2.f, 2.f, 2.f};
    faceTransform->_Position = {-1.f, -1.f, 0.f};

    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = faceModel}, 
        {._Transform = faceTransform}
    );
    _GameObjects.push_back(object);


    // be::TransformPtr transform = be::TransformPtr(
    //     new be::Transform()
    // );
    // transform->_Position = {-1.f, -1.f, 0.f};

    // object = be::RenderSystem::createRenderableObject(
    //     {._RenderSubSystem = _BRDFRenderSubSystem},
    //     {._Model = faceModel},
    //     {._Transform = transform}
    // );
    // _GameObjects.push_back(object);

}
void Application::initCamera(){
    _Camera = be::CameraPtr(new be::Camera(be::Vector3(0.f,0.f,4.f)));
}
void Application::initRenderer(){
    if(_Window == nullptr){
        be::ErrorHandler::handle(__FILE__, __LINE__, 
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a renderer without a window!\n"
        );
    }
    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(__FILE__, __LINE__, 
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
        be::ErrorHandler::handle(__FILE__, __LINE__, 
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a render subsystem without a renderer!\n"
        );
    }
    if(_Camera == nullptr){
        be::ErrorHandler::handle(__FILE__, __LINE__, 
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a render subsystem without a camera!\n"
        );
    }
    _RenderSubSystem = FrameRenderSubSystemPtr(
                        new FrameRenderSubSystem(
                            _VulkanApp, 
                            _Renderer->getSwapChainRenderPass(),
                            _GlobalPool
                            )
                        );

    _BRDFRenderSubSystem = BrdfRenderSubSystemPtr(
                        new BrdfRenderSubSystem(
                            _VulkanApp, 
                            _Renderer->getSwapChainRenderPass(),
                            _GlobalPool
                            )
                        );
    
    _RaytracingRenderSubSystem = RaytracingRenderSubSystemPtr(
                        new RaytracingRenderSubSystem(
                            _VulkanApp, 
                            _Renderer->getSwapChainRenderPass(),
                            _GlobalPoolTmp
                            )
                        );
}
void Application::initDescriptors(){

    uint32_t nbRasterizerSets = FrameRenderSubSystem::_NB_SETS + BrdfRenderSubSystem::_NB_SETS;
    uint32_t nbRaytracerSets = RaytracingRenderSubSystem::_NB_SETS;

    _GlobalPool = be::DescriptorPool::Builder(_VulkanApp)
        .setMaxSets(nbRasterizerSets*be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nbRasterizerSets*be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .build();

    _GlobalPoolTmp = be::DescriptorPool::Builder(_VulkanApp)
        .setMaxSets(nbRaytracerSets*be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nbRaytracerSets*be::SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT)
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
void Application::cleanUpRenderSubSystems(){
    _RenderSubSystem->cleanUp();
    _BRDFRenderSubSystem->cleanUp();
    _RaytracingRenderSubSystem->cleanUp();
}
void Application::cleanUpDescriptors(){
    _GlobalPool->cleanUp();
    _GlobalPoolTmp->cleanUp();
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

        float height = _Renderer->getSwapChain()->getHeight();
        float width = _Renderer->getSwapChain()->getWidth();
        _RayTracer->setResolution(width, height);
        _Camera->setAspectRatio(width, height);
        _Camera->setDt(frameTime);

        KeyboardInput::updateMouseMode(_Window);
        KeyboardInput::switchRenderingMode(_Window, this);
        if(_Window->wasWindowResized()){
            _RayTracer->setResolution(_Renderer->getSwapChain()->getWidth(), _Renderer->getSwapChain()->getHeight());
        }

        switch(_RenderingMode){
            case RAY_TRACING:{
                _Camera->lock();
                KeyboardInput::runRaytracer(_Window, this);
                // IMGUI
                {
                    // Start the Dear ImGui frame
                    ImGui_ImplVulkan_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    // modify materials values
                    ImGui::Begin("Render commands");
                    ImGui::Text("Switch to Rasterizing: TAB");
                    ImGui::Text("Run ray tracer: SPACE");
                    ImGui::End();
                }

                // Rendering
                ImGui::Render();
                ImDrawData* draw_data = ImGui::GetDrawData();

                auto commandBuffer = _Renderer->beginFrame();
                if(commandBuffer){
                    _CurrentFrame._FrameIndex = _Renderer->getFrameIndex();
                    _CurrentFrame._FrameTime = frameTime;
                    _CurrentFrame._CommandBuffer = commandBuffer;
                    _CurrentFrame._Camera = _Camera;

                    _Renderer->beginSwapChainRenderPass(commandBuffer);
                    if(_RaytracingRenderSubSystem->isInit()){
                        _RaytracingRenderSubSystem->renderGameObjects(_CurrentFrame);
                    }
                    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

                    _Renderer->endSwapChainRenderPass(commandBuffer);
                    _Renderer->endFrame();
                }
                break;
            }

            case RASTERIZING:{
                _Hasrun = false;
                _Camera->unlock();
                KeyboardInput::switchPipeline(_Window, _BRDFRenderSubSystem);
                KeyboardInput::moveCamera(_Window, _Camera);
                _Scene->setLights(_BRDFRenderSubSystem->getPointLights(), _BRDFRenderSubSystem->getDirectionalLights());

                // IMGUI
                {
                    // Start the Dear ImGui frame
                    ImGui_ImplVulkan_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    // modify materials values
                    ImGui::Begin("Material values");
                    auto& material = be::GameCoordinator::getComponent<be::ComponentMaterial>(_GameObjects[1]);
                    for(uint32_t i=0; i<be::Material::COMPONENT_MATERIAL_NB_ELEMENTS; i++){
                        ImGui::SliderFloat(
                            be::Material::COMPONENT_MATERIAL_NAMES[i].c_str(),
                            &material._Material->get(i),
                            be::Material::COMPONENT_MATERIAL_MIN_VALUES[i],
                            be::Material::COMPONENT_MATERIAL_MAX_VALUES[i]
                        );
                    }
                    ImGui::End();
                }

                // Rendering
                ImGui::Render();
                ImDrawData* draw_data = ImGui::GetDrawData();

                auto commandBuffer = _Renderer->beginFrame();
                if(commandBuffer){
                    _CurrentFrame._FrameIndex = _Renderer->getFrameIndex();
                    _CurrentFrame._FrameTime = frameTime;
                    _CurrentFrame._CommandBuffer = commandBuffer;
                    _CurrentFrame._Camera = _Camera;

                    _Renderer->beginSwapChainRenderPass(commandBuffer);

                    _RenderSubSystem->renderGameObjects(_CurrentFrame);
                    _BRDFRenderSubSystem->renderGameObjects(_CurrentFrame);
                    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

                    _Renderer->endSwapChainRenderPass(commandBuffer);
                    _Renderer->endFrame();
                }
                break;
            }
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}