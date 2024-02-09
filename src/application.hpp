#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

#include "types.hpp"
#include "window.hpp"
#include "pipeline.hpp"

#include "renderSystem.hpp"
#include "renderer.hpp"

struct SimplePushConstantData{
    alignas(4) float _Random;
    alignas(16) glm::mat4 _Model;
};

class Application{
    friend RenderSystem;

    public:
        static const uint32_t WINDOW_WIDTH = 800;
        static const uint32_t WINDOW_HEIGHT = 600;

    private:
        WindowPtr _Window = nullptr;
        VulkanAppPtr _VulkanApp = nullptr;
        RendererPtr _Renderer = nullptr;

        PipelinePtr _Pipeline = nullptr;
        VkPipelineLayout _PipelineLayout;

        // ECS test
        RenderSystemPtr _RenderSystem = nullptr;
        std::vector<GameObject> _GameObjects{};

    private:
        void initWindow();
        void cleanUpWindow();

        void initVulkan();
        void cleanUpVulkan();

        void initGameObjects();
        void cleanUpGameObjects();

        void initRenderer();
        void cleanUpRenderer();

        void initPipelineLayout();
        void cleanUpPipelineLayout();

        void initPipeline();
        void cleanUpPipeline();

    private:
        void init(){
            initWindow();
            initVulkan();
            initRenderer();
            initPipelineLayout();
            initPipeline();
            initGameObjects();
        }
        void cleanUp(){
            cleanUpGameObjects();
            cleanUpPipeline();
            cleanUpPipelineLayout();
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