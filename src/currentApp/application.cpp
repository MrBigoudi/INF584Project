#include "applicationTest.hpp"

#include <chrono>
#include "keyboardInput.hpp"

/********************************************************************/
/************************** INIT FUNCTIONS **************************/
/********************************************************************/
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
void Application::initGameObjectsFrame(){
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
}
void Application::initGameObjectsRayTracingViewRectangle(){
    be::ModelPtr viewportRectangleModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            2.f, 
            2.f
            )
        )
    );

    be::GameObject object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _RaytracingRenderSubSystem},
        {._Model = viewportRectangleModel}
    );
    _GameObjects.push_back(object);
}
void Application::initGameObjectsRoom(){
    // create the room by hand for faster raytracing
    be::ModelPtr floorModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            1.f, 1.f, {0.f, 1.f, 0.f, 1.f}
        ))
    );
    be::TransformPtr floorTransform = be::TransformPtr(new be::Transform());
    floorTransform->_Scale = {15.f, 15.f, 1.f};
    floorTransform->_Rotation = {be::radians(-90.f), 0.f, 0.f};
    floorTransform->_Position = {0.f, -7.5f, 0.f};
    be::GameObject object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = floorModel},
        {._Transform = floorTransform}
    );
    _GameObjects.push_back(object);

    be::ModelPtr roofModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            1.f, 1.f, {1.f, 1.f, 1.f, 1.f}
        ))
    );
    be::TransformPtr roofTransform = be::TransformPtr(new be::Transform());
    roofTransform->_Scale = {15.f, 15.f, 1.f};
    roofTransform->_Rotation = {be::radians(90.f), 0.f, 0.f};
    roofTransform->_Position = {0.f, 7.5f, 0.f};
    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = roofModel},
        {._Transform = roofTransform}
    );
    _GameObjects.push_back(object);

    be::ModelPtr frontWallModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            1.f, 1.f, {0.01f, 0.01f, 0.01f, 1.f}
        ))
    );
    be::TransformPtr frontWallTransform = be::TransformPtr(new be::Transform());
    frontWallTransform->_Scale = {15.f, 15.f, 1.f};
    frontWallTransform->_Rotation = {0.f, be::radians(180.f), 0.f};
    frontWallTransform->_Position = {0.f, 0.f, 7.5f};
    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = frontWallModel},
        {._Transform = frontWallTransform}
    );
    _GameObjects.push_back(object);

    be::ModelPtr backWallModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            1.f, 1.f, {0.01f, 0.01f, 0.01f, 1.f}
        ))
    );
    be::TransformPtr backWallTransform = be::TransformPtr(new be::Transform());
    backWallTransform->_Scale = {15.f, 15.f, 1.f};
    backWallTransform->_Rotation = {0.f, 0.f, 0.f};
    backWallTransform->_Position = {0.f, 0.f, -7.5f};
    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = backWallModel},
        {._Transform = backWallTransform}
    );
    _GameObjects.push_back(object);

    be::ModelPtr leftWallModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            1.f, 1.f, {1.f, 0.f, 0.f, 1.f}
        ))
    );
    be::TransformPtr leftWallTransform = be::TransformPtr(new be::Transform());
    leftWallTransform->_Scale = {15.f, 15.f, 1.f};
    leftWallTransform->_Rotation = {0.f, be::radians(90.f), 0.f};
    leftWallTransform->_Position = {-7.5f, 0.f, 0.f};
    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = leftWallModel},
        {._Transform = leftWallTransform}
    );
    _GameObjects.push_back(object);

    be::ModelPtr rightWallModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveRectangle(
            1.f, 1.f, {0.f, 0.f, 1.f, 1.f}
        ))
    );
    be::TransformPtr rightWallTransform = be::TransformPtr(new be::Transform());
    rightWallTransform->_Scale = {15.f, 15.f, 1.f};
    rightWallTransform->_Rotation = {0.f, be::radians(-90.f), 0.f};
    rightWallTransform->_Position = {7.5f, 0.f, 0.f};
    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = rightWallModel},
        {._Transform = rightWallTransform}
    );
    _GameObjects.push_back(object);
}
void Application::initDragonScene(){
    // load dragon model
    be::ModelPtr dragonModel = be::ModelPtr(
        new be::Model(_VulkanApp, "resources/models/dragon.off")
    );


    be::TransformPtr dragonTransform = be::TransformPtr(
        new be::Transform()
    );
    dragonTransform->_Scale = {10.f, 10.f, 10.f};
    be::MaterialPtr sphereMaterial = be::MaterialPtr(
        new be::Material(
            {
                ._Metallic = 0.788f,
                ._Subsurface = 0.603f,
                ._Specular = 0.301f,
                ._Roughness = 0.180f,
                ._SpecularTint = 0.042f,
                ._Anisotropic = 0.199f,
                ._Sheen = 0.564f,
                ._SheenTint = 0.795f,
                ._Clearcoat = 0.269f,
                ._ClearcoatGloss = 0.737f
            }
        )
    );

    be::GameObject object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = dragonModel}, 
        {._Transform = dragonTransform},
        {._Material = sphereMaterial, ._MaterialId = 1}
    );
    _GameObjects.push_back(object);
}
void Application::initSpheresScene(){
    // load sphere model
    be::ModelPtr sphereModel = be::ModelPtr(
        new be::Model(_VulkanApp, be::VertexDataBuilder::primitiveSphere(16, 
            {54.f/255.f, 112.f/255.f, 131.f/255.f})
        )
    );


    be::TransformPtr sphereTransform = be::TransformPtr(
        new be::Transform()
    );
    sphereTransform->_Scale = {2.f, 2.f, 2.f};
    be::MaterialPtr sphereMaterial = be::MaterialPtr(new be::Material());

    be::GameObject object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = sphereModel}, 
        {._Transform = sphereTransform},
        {._Material = sphereMaterial, ._MaterialId = 1}
    );
    _GameObjects.push_back(object);
    
    be::TransformPtr sphereTransform2 = be::TransformPtr(
        new be::Transform()
    );
    sphereTransform2->_Scale = {2.f, 2.f, 2.f};
    sphereTransform2->_Position = {3.f, 3.f, 0.f};
    object = be::RenderSystem::createRenderableObject(
        {._RenderSubSystem = _BRDFRenderSubSystem},
        {._Model = sphereModel}, 
        {._Transform = sphereTransform2}
    );
    _GameObjects.push_back(object);
}
void Application::initGameObjectsEntities(){
    initDragonScene();
    // initSpheresScene();
}
void Application::initGameObjects(){
    if(_VulkanApp == nullptr){
        be::ErrorHandler::handle(__FILE__, __LINE__, 
            be::ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create game objects without a vulkan app!\n"
        );
    }
    initGameObjectsFrame();
    initGameObjectsRayTracingViewRectangle();
    initGameObjectsEntities();
    initGameObjectsRoom();

    // _Scene->addGameObject(0);
    // add all objects except the frame and the raytracing view rectangle
    for(uint32_t i=2; i<_GameObjects.size(); i++){
        _Scene->addGameObject(_GameObjects[i]);
    }
}
void Application::initLightsBasic(){
    // build the lights
    _Scene->addGamePointLight(
        {0.f, 7.f, -1.f}, 
        {1.f, 1.f, 1.f},
        1.f 
    );
    _Scene->addGamePointLight(
        {0.f, 7.f, 1.f}, 
        {1.f, 1.f, 1.f},
        1.f 
    );
    _Scene->addGamePointLight(
        {7.f, 0.f, -1.f}, 
        {1.f, 1.f, 1.f},
        1.f 
    );
    _Scene->addGamePointLight(
        {7.f, 0.f, 1.f}, 
        {1.f, 1.f, 1.f},
        1.f 
    );

    // make the lights visible
    for(auto light: _Scene->getPointLights()){
        be::ModelPtr lightModel = be::ModelPtr(
            new be::Model(_VulkanApp, 
                be::VertexDataBuilder::primitiveSphere(16, 
                    be::Vector4(light->getColor(), 1.f)
                )
            )
        );
        be::TransformPtr lightTransform = be::TransformPtr(
            new be::Transform()
        );
        lightTransform->_Scale = {0.1f, 0.1f, 0.1f};
        lightTransform->_Position = light->_Position.xyz();
        be::GameObject object = be::RenderSystem::createRenderableObject(
            {._RenderSubSystem = _RenderSubSystem},
            {._Model = lightModel}, 
            {._Transform = lightTransform},
            {},
            {._IsLight = true}
        );
        _GameObjects.push_back(object);
        _Scene->addGameObject(object);
    }
}
void Application::initLightsCircle(){
    // light in a circle
    float r = 7.f;
    int nbLights = 36;
    float step = be::radians(360.f / nbLights);
    float curAngle = 0.f;
    for(int i=0; i<nbLights; i++){
        be::Vector3 pos = {r*std::cos(curAngle), -3.f, r*std::sin(curAngle)};
        be::Vector3 col = be::Vector3::random(0.f, 1.f);
        // be::Vector3 col = {1.f, 1.f, 1.f};
        float intensity = 5.f;
        _Scene->addGamePointLight(
            pos,
            col,
            intensity
        );
        curAngle += step;
    }



    // make the lights visible
    for(auto light: _Scene->getPointLights()){
        be::ModelPtr lightModel = be::ModelPtr(
            new be::Model(_VulkanApp, 
                be::VertexDataBuilder::primitiveSphere(16, 
                    be::Vector4(light->getColor(), 1.f)
                )
            )
        );
        be::TransformPtr lightTransform = be::TransformPtr(
            new be::Transform()
        );
        lightTransform->_Scale = {0.1f, 0.1f, 0.1f};
        lightTransform->_Position = light->_Position.xyz();
        be::GameObject object = be::RenderSystem::createRenderableObject(
            {._RenderSubSystem = _RenderSubSystem},
            {._Model = lightModel}, 
            {._Transform = lightTransform},
            {},
            {._IsLight = true}
        );
        _GameObjects.push_back(object);
        _Scene->addGameObject(object);
    }
}
void Application::initLightsBoxes(){
    float roomHalfSize = 7.5;

    float lightSteps = 0.5;

    // bottom right cube
    float cube1Length = 4.f;
    be::Vector3 cube1Center = be::Vector3(5.f, -roomHalfSize+cube1Length/2.f, -5.f);
    be::Vector3 cube1Rotation = be::Vector3(0.f, be::radians(-15.f), 0.f);
    be::Vector3 cube1Color = be::Vector3(1.f, 1.f, 0.5f);
    // be::Vector3 cube1Color = be::Color::WHITE;
    float cube1Intensity = 0.8f;
    auto object = _Scene->addCubeOfLight(
        _RenderSubSystem, 
        cube1Center,
        cube1Length, 
        cube1Color,
        cube1Intensity,
        cube1Rotation,
        lightSteps
    );
    _GameObjects.push_back(object);

    // tiny bottom right cube
    float cube2Length = 2.f;
    be::Vector3 cube2Center = be::Vector3(4.f, -roomHalfSize+cube2Length/2.f, 3.f);
    be::Vector3 cube2Rotation = be::Vector3(0.f, be::radians(45.f), 0.f);
    be::Vector3 cube2Color = be::Vector3(73.f/255.f, 116.f/255.f, 165.f/255.f);
    // be::Vector3 cube2Color = be::Color::WHITE;
    float cube2Intensity = 0.2f;
    object = _Scene->addCubeOfLight(
        _RenderSubSystem, 
        cube2Center,
        cube2Length, 
        cube2Color,
        cube2Intensity,
        cube2Rotation,
        lightSteps
    );
    _GameObjects.push_back(object);

    // bottom left cube
    float cube3Length = 3.f;
    be::Vector3 cube3Center = be::Vector3(-5.2f, -roomHalfSize+cube3Length/2.f, 1.5f);
    be::Vector3 cube3Rotation = be::Vector3(0.f, be::radians(30.f), 0.f);
    be::Vector3 cube3Color = be::Vector3(122.f/255.f, 73.f/255.f, 165.f/255.f);
    // be::Vector3 cube3Color = be::Color::WHITE;
    float cube3Intensity = 0.6f;
    object = _Scene->addCubeOfLight(
        _RenderSubSystem, 
        cube3Center,
        cube3Length, 
        cube3Color,
        cube3Intensity,
        cube3Rotation,
        lightSteps
    );
    _GameObjects.push_back(object);


    // top light
    be::Vector3 cube4Scale = be::Vector3(6.f, 0.3f, 6.f);
    be::Vector3 cube4Center = be::Vector3(0.f, roomHalfSize-cube4Scale.y()/2.f, 0.f);
    be::Vector3 cube4Color = be::Color::WHITE;
    float cube4Intensity = 5.f;
    object = _Scene->addCubeOfLight(
        _RenderSubSystem, 
        cube4Center,
        cube4Scale, 
        cube4Color,
        cube4Intensity,
        be::Vector3::zeros(),
        be::Vector3(lightSteps, 0.1f, lightSteps)
    );
    _GameObjects.push_back(object);
}
void Application::initLights(){
    // initLightsBasic();
    initLightsCircle();
    initLightsBoxes();
}
void Application::initCamera(){
    _Camera = be::CameraPtr(new be::Camera(be::Vector3(0.f, 0.f, 20.f)));
    // _Camera = be::CameraPtr(new be::Camera(be::Vector3(0.f, 0.f, 15.f)));
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
void Application::initScene(){
    _Scene = be::ScenePtr(new be::Scene(_VulkanApp));
    _BRDFRenderSubSystem->setScene(_Scene);
}
void Application::initRaytracer(){
    _RayTracer = be::RayTracerPtr(
        new be::RayTracer(
            _Scene, 
            _Renderer->getSwapChain()->getWidth(), 
            _Renderer->getSwapChain()->getHeight()
        )
    );
}
void Application::initGUI(){
    MouseInput::setMouseCallback(_Camera, _Window);
    // init imgui after setting up the callbacks
    be::BeImgui::init(_Window, _VulkanApp, _Renderer);
}





/*******************************************************************/
/************************ CLEANUP FUNCTIONS ************************/
/*******************************************************************/
void Application::cleanUpGUI(){
    be::BeImgui::cleanUp(_VulkanApp);
}
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





/*******************************************************************/
/************************ HELPER FUNCTIONS *************************/
/*******************************************************************/
void Application::updateSwitchRenderingModeKey(){
    if(!_IsSwitchRenderingModeKeyPressed){
        _IsSwitchRenderingModeKeyPressed = true;
        switchRenderingMode();
    }
}
void Application::resetSwitchRenderingModeKey(){
    _IsSwitchRenderingModeKeyPressed = false;
}
void Application::switchRenderingMode(){
    switch(_RenderingMode){
        case RASTERIZING:
            _RenderingMode = RAY_TRACING;
            break;
        case RAY_TRACING:
            _RenderingMode = RASTERIZING;
            break;
    }
}
void Application::runRaytracer(){
    if(_RenderingMode == RAY_TRACING && !_Hasrun){
        be::Vector3 backgroundColor = be::Color::toSRGB({0.383f, 0.632f, 0.800f});
        // to match brdf render sub system
        switch(_BRDFRenderSubSystem->getBRDFModel()){
            case COLOR_BRDF:
                _RayTracer->enableColorBRDF();
                break;
            case NORMAL_BRDF:
                _RayTracer->enableNormalBRDF();
                break;
            case LAMBERT_BRDF:
                _RayTracer->enableLambertBRDF();
                break;
            case MICROFACET_BRDF:
                _RayTracer->enableGgxBRDF();
                break;
            default:
                break;
        }

        _RayTracer->run(_CurrentFrame, backgroundColor);
        if(_SaveImage){
            _RayTracer->getImage()->savePPM();
        }
        _RaytracingRenderSubSystem->setRenderPass(_Renderer->getSwapChainRenderPass());
        _RaytracingRenderSubSystem->updateImage(_RayTracer->getImage());
        _Hasrun = true;
    }
}



/*******************************************************************/
/************************* MAIN FUNCTIONS **************************/
/*******************************************************************/
void Application::run(){
    static const uint32_t SEED = 4242;
    srand(SEED);
    init();
    mainLoop();
    cleanUp();
}

void Application::init() {
    initWindow();
    initVulkan();
    initCamera();
    be::Components::registerComponents();
    initRenderer();
    initDescriptors();
    initSystems();
    initRenderSubSystems();
    initScene();
    initRaytracer();
    initGameObjects();
    initLights();
    initGUI();
}

void Application::cleanUp(){
    cleanUpGUI();
    cleanUpGameObjects();
    cleanUpRenderSubSystems();
    cleanUpDescriptors();
    cleanUpRenderer();
    cleanUpVulkan();
    cleanUpWindow();
}

void Application::renderRayTracing(float frameTime){
    _Camera->lock();
    KeyboardInput::runRaytracer(_Window, this);
    // IMGUI
    {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // modify raytracer values
        ImGui::Begin("Render commands");
        ImGui::Text("Switch to Rasterizing: TAB");
        ImGui::Text("Run ray tracer: SPACE");

        ImGui::Checkbox(
            "Save result image", 
            &_SaveImage
        );

        // ray tracer parameters
        ImGui::Text("Raytracer parameters:\n");
        ImGui::SliderInt(
            "Samples per pixels", 
            reinterpret_cast<int*>(&_RayTracer->_SamplesPerPixels), 
            1, 
            64
        );

        ImGui::SliderInt(
            "Max bounces", 
            reinterpret_cast<int*>(&_RayTracer->_MaxBounces), 
            0, 
            5
        );

        ImGui::SliderInt(
            "Samples per bounces", 
            reinterpret_cast<int*>(&_RayTracer->_SamplesPerBounces), 
            1, 
            64
        );

        ImGui::SliderFloat(
            "Shading factor for bounces",
            &_RayTracer->_ShadingFactor,
            0.f,
            1.f
        );

        // lightcuts parameters
        ImGui::Text("Lightcuts parameters:\n");
        ImGui::Checkbox(
            "Use lightcuts", 
            &_RayTracer->_UseLightCuts
        );

        ImGui::SliderFloat(
            "Error threshold",
            &_RayTracer->_LightcutsErrorThreshold,
            0.f,
            0.5f
        );

        ImGui::SliderInt(
            "Maximum size of a cut", 
            reinterpret_cast<int*>(&_RayTracer->_LightcutsMaxClusters), 
            1, 
            200
        );


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
}
void Application::renderRasterizer(float frameTime){
    _Hasrun = false;
    _Camera->unlock();
    KeyboardInput::switchPipeline(_Window, _BRDFRenderSubSystem);
    KeyboardInput::moveCamera(_Window, _Camera);

    // IMGUI
    {
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // modify materials values
        ImGui::Begin("Render commands");
        ImGui::Text("Switch to Rasterizing: TAB");
        ImGui::Text("Switch Pipeline: P (current %s)", 
            BrdfRenderSubSystem::_PIPELINE_NAMES[_BRDFRenderSubSystem->getBRDFModel()].c_str());
        ImGui::Text("Toogle Wireframe: F1");
        ImGui::Text("\nMaterial properties:\n");
        auto& material = be::GameCoordinator::getComponent<be::ComponentMaterial>(
            _GameObjects[2]
        );
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
}

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
            case RAY_TRACING:
                renderRayTracing(frameTime);
                break;
            case RASTERIZING:
                renderRasterizer(frameTime);
                break;
        }
    }
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}