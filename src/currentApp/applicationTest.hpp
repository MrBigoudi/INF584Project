#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <engine.hpp>

#include "keyboardInput.hpp"
#include "mouseInput.hpp"

class Application;
using ApplicationPtr = std::shared_ptr<Application>;

class Application : public BE::IApplication{

    public:
        static const uint32_t WINDOW_WIDTH = 1280;
        static const uint32_t WINDOW_HEIGHT = 720;

    private:
        BE::DescriptorPoolPtr _GlobalPool = nullptr;
        BE::DescriptorPoolPtr _GlobalPoolTmp = nullptr;

        BE::SimpleRenderSubSystemPtr _RenderSubSystem = nullptr;
        BE::GlobalFrameRenderSubSystemPtr _GlobalFrameRenderSubSystem = nullptr;

        BE::CameraPtr _Camera = nullptr;

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
            BE::Components::registerComponents();
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