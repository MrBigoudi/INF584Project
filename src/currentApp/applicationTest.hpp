#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <BigoudiEngine.hpp>

#include "renderSubSystems.hpp" // IWYU pragma: keep

#include "mouseInput.hpp"

class Application;
using ApplicationPtr = std::shared_ptr<Application>;

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

        // TODO: it is just a test
        // be::ScenePtr _Scene = nullptr;

    public:
        static const uint32_t _NB_SETS = 
            FrameRenderSubSystem::_NB_SETS
            + BrdfRenderSubSystem::_NB_SETS
        ;

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
            initGameObjects();
            MouseInput::setMouseCallback(_Camera, _Window);
            // init imgui after setting up the callbacks
            be::BeImgui::init(_Window, _VulkanApp, _Renderer);

            // _Scene = be::ScenePtr(new be::Scene(_VulkanApp));
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

};