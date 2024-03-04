#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <BigoudiEngine.hpp>

#include "renderSubSystems.hpp" // IWYU pragma: keep

#include "mouseInput.hpp"

class Application;
using ApplicationPtr = std::shared_ptr<Application>;

enum RenderingMode{
    RASTERIZING,
    RAY_TRACING,
};

class Application : public be::IApplication{

    public:
        static const uint32_t WINDOW_WIDTH = 1280;
        static const uint32_t WINDOW_HEIGHT = 720;

    private:
        be::DescriptorPoolPtr _GlobalPool = nullptr;
        be::DescriptorPoolPtr _GlobalPoolTmp = nullptr;

        std::vector<be::GameObject> _GameObjects = {};
        be::CameraPtr _Camera = nullptr;

        FrameRenderSubSystemPtr _RenderSubSystem = nullptr;
        BrdfRenderSubSystemPtr _BRDFRenderSubSystem = nullptr;
        RaytracingRenderSubSystemPtr _RaytracingRenderSubSystem = nullptr;

        // TODO: it is just a test
        be::ScenePtr _Scene = nullptr;
        bool _IsSwitchRenderingModeKeyPressed = false;
        RenderingMode _RenderingMode = RASTERIZING;
        be::RayTracerPtr _RayTracer = nullptr;

        

    private:
        void initSystems();

        void initWindow();
        void cleanUpWindow();

        void initVulkan();
        void cleanUpVulkan();

        void initGameObjects();
        void cleanUpGameObjects();
        void initCamera();

        void initRenderer();
        void cleanUpRenderer();

        void initRenderSubSystems();
        void cleanUpRenderSubSystems();

        void initDescriptors();
        void cleanUpDescriptors();

    private:
        void init() override {
            initWindow();
            initVulkan();
            initCamera();
            be::Components::registerComponents();
            initRenderer();
            initDescriptors();
            initSystems();
            initRenderSubSystems();

            _Scene = be::ScenePtr(new be::Scene(_VulkanApp));
            // TODO: switch to use swap chain width / height
            _RayTracer = be::RayTracerPtr(new be::RayTracer(_Scene, _Camera, WINDOW_WIDTH, WINDOW_HEIGHT));
            initGameObjects();
            _Scene->addGameObject(_GameObjects[1]);

            MouseInput::setMouseCallback(_Camera, _Window);
            // init imgui after setting up the callbacks
            be::BeImgui::init(_Window, _VulkanApp, _Renderer);

            // _Scene->initFromGLTF("./src/engine/resources/models/glTF/scenes/Sponza/glTF/Sponza.gltf");
            // for(auto obj: _Scene->getObjects()){
            //     be::ComponentRenderSubSystem::add(obj, _BRDFRenderSubSystem);
            //     be::ComponentMaterial::add(obj);
            //     _GameObjects.push_back(obj);
            // }
        }
        void cleanUp() override {
            be::BeImgui::cleanUp(_VulkanApp);
            cleanUpGameObjects();
            cleanUpRenderSubSystems();
            cleanUpDescriptors();
            cleanUpRenderer();
            cleanUpVulkan();
            cleanUpWindow();
        }

        void mainLoop() override;

    public:
        Application(){};

        void run() override{
            init();
            mainLoop();
            cleanUp();
        }

        void updateSwitchRenderingModeKey(){
            if(!_IsSwitchRenderingModeKeyPressed){
                _IsSwitchRenderingModeKeyPressed = true;
                switchRenderingMode();
            }
        }

        void resetSwitchRenderingModeKey(){
            _IsSwitchRenderingModeKeyPressed = false;
        }

        void switchRenderingMode(){
            switch(_RenderingMode){
                case RASTERIZING:
                    _RenderingMode = RAY_TRACING;
                    break;
                case RAY_TRACING:
                    _RenderingMode = RASTERIZING;
                    break;
            }
        }

        void runRaytracer(){
            if(_RenderingMode == RAY_TRACING){
                _RayTracer->run({0.383f, 0.632f, 0.800f}, true);
                _RayTracer->getImage()->savePPM("tmp/ray_tracer.ppm");
                _RaytracingRenderSubSystem->updateImage(_RayTracer->getImage());
            }
        }

};