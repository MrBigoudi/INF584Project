#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <BigoudiEngine.hpp>

#include "keyboardInput.hpp"
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

        be::CameraPtr _Camera = nullptr;

    private:
        void initWindow();
        void cleanUpWindow();

        void initVulkan();
        void cleanUpVulkan();

        void initGameObjects();
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
            initRenderSubSystems();
            initGameObjects();

            MouseInput::setMouseCallback(_Camera, _Window);
        }
        void cleanUp() override {
            cleanUpDescriptors();
            cleanUpRenderSubSystems();
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