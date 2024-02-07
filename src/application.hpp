#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>

#include "types.hpp"
#include "window.hpp"
#include "pipeline.hpp"
#include "swapChain.hpp"
#include "model.hpp"

struct SimplePushConstantData{
    alignas(4) float _Random;
};

class Application{
    public:
        static const uint32_t WINDOW_WIDTH = 800;
        static const uint32_t WINDOW_HEIGHT = 600;

    private:
        WindowPtr _Window = nullptr;
        VulkanAppPtr _VulkanApp = nullptr;
        SwapChainPtr _SwapChain = nullptr;

        ModelPtr _Model = nullptr;

        PipelinePtr _Pipeline = nullptr;
        VkPipelineLayout _PipelineLayout;
        std::vector<VkCommandBuffer> _CommandBuffers;

    private:
        void init();
        void initWindow();
        void initVulkan();
        void initSwapChain();
        void initPipeline();
        void initPipelineLayout();
        void initCommandBuffers();

        void initModels();

        void mainLoop();
        void cleanUp();

        void cleanUpCommandBuffers();
        void drawFrame();

        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

    public:
        Application(){};
        void run();
        WindowPtr getWindow() const {
            return _Window;
        }
};