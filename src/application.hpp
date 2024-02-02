#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>

#include "types.hpp"
#include "window.hpp"
#include "pipeline.hpp"

class Application{
    public:
        static const uint32_t WINDOW_WIDTH = 800;
        static const uint32_t WINDOW_HEIGHT = 600;

    private:
        WindowPtr _Window = nullptr;
        VulkanAppPtr _VulkanApp = nullptr;
        PipelinePtr _Pipeline = nullptr;

    private:
        void init();
        void initWindow();
        void initVulkan();
        void initPipeline();

        void mainLoop();
        void cleanUp();
        void drawFrame();

    public:
        Application();
        void run();
        WindowPtr getWindow() const {
            return _Window;
        }
};