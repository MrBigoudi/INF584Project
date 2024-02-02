#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>

#include "types.hpp"
#include "window.hpp"

class Application{
    public:
        static const uint32_t WINDOW_WIDTH = 800;
        static const uint32_t WINDOW_HEIGHT = 600;

    private:
        WindowPtr _Window = nullptr;
        VulkanAppPtr _VulkanApp = nullptr;

    private:
        void init();
        void initWindow();
        void initVulkan();

        void mainLoop();
        void cleanUp();
        void drawFrame();

    public:
        Application();
        void run();
};