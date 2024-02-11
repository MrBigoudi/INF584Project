#pragma once

#include "mouseInput.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <vector>

#include "types.hpp"
#include "window.hpp"

#include "renderer.hpp"

#include "simpleRenderSubSystem.hpp"
#include "camera.hpp"


class Application{

    public:
        static const uint32_t WINDOW_WIDTH = 800;
        static const uint32_t WINDOW_HEIGHT = 600;

    private:
        WindowPtr _Window = nullptr;
        VulkanAppPtr _VulkanApp = nullptr;
        RendererPtr _Renderer = nullptr;

        SimpleRenderSubSystemPtr _RenderSubSystem = nullptr;

        CameraPtr _Camera = nullptr;

    private:
        void initWindow();
        void cleanUpWindow();

        void initVulkan();
        void cleanUpVulkan();

        void initGameObjects();
        void initCamera();

        void initRenderer();
        void cleanUpRenderer();

        void initRenderSubSystem();
        void cleanUpRenderSubSystem();

    private:
        void init(){
            initWindow();
            initVulkan();
            initCamera();
            initRenderer();
            initRenderSubSystem();
            initGameObjects();
            MouseInput::setMouseCallback(_Camera, _Window);
        }
        void cleanUp(){
            cleanUpRenderSubSystem();
            cleanUpRenderer();
            cleanUpVulkan();
            cleanUpWindow();
        }

        void mainLoop();

    public:
        Application(){};

        void run(){
            init();
            mainLoop();
            cleanUp();
        }

        WindowPtr getWindow() const {
            return _Window;
        }
};