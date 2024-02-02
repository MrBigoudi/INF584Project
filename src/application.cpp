#include <iostream>

#include "application.hpp"
#include "pipeline.hpp"
#include "types.hpp"
#include "vulkanApp.hpp"
#include "window.hpp"

void Application::init(){
    initWindow();
    initVulkan();
    initPipeline();
}

void Application::initVulkan(){
    _VulkanApp = VulkanAppPtr(new VulkanApp());
    _VulkanApp->init(ApplicationPtr(this));
}

void Application::mainLoop(){
    while(!_Window->shouldClose()){
        glfwPollEvents();
        drawFrame();
    }
}

void Application::cleanUp(){
    _VulkanApp->cleanUp();
    _Window->cleanUp();
}

void Application::drawFrame(){
}

Application::Application(){
}

void Application::run(){
    init();
    mainLoop();
    cleanUp();
}

void Application::initWindow(){
    _Window = WindowPtr(
        new Window(WINDOW_WIDTH, WINDOW_HEIGHT, "hello vulkan")
    );
    _Window->init();
}

void Application::initPipeline(){
    _Pipeline = PipelinePtr(
        new Pipeline(
            "shaders/compiled/basicTriangleVert.spv",
            "shaders/compiled/basicTriangleFrag.spv"
        )
    );
}