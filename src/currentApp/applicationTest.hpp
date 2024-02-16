#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <BigoudiEngine.hpp>

#include "keyboardInput.hpp"
#include "mouseInput.hpp"

class Application;
using ApplicationPtr = std::shared_ptr<Application>;

class Application : public beCore::IApplication{

    public:
        static const uint32_t WINDOW_WIDTH = 1280;
        static const uint32_t WINDOW_HEIGHT = 720;

    private:
        beCore::DescriptorPoolPtr _GlobalPool = nullptr;
        beCore::DescriptorPoolPtr _GlobalPoolTmp = nullptr;

        beCore::SimpleRenderSubSystemPtr _RenderSubSystem = nullptr;
        beCore::GlobalFrameRenderSubSystemPtr _GlobalFrameRenderSubSystem = nullptr;

        beCore::CameraPtr _Camera = nullptr;

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
            beCore::Components::registerComponents();
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