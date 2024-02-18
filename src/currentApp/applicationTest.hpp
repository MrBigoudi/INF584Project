#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <BigoudiEngine.hpp>

#include "renderSubSystems.hpp"

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

        SimpleRenderSubSystemPtr _RenderSubSystem = nullptr;
        NormalRenderSubSystemPtr _NormalRenderSubSystem = nullptr;
        std::vector<be::GameObject> _GameObjects = {};

        be::CameraPtr _Camera = nullptr;

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
        }
        void cleanUp() override {
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