#include <iostream>
#include <vulkan/vulkan_core.h>

#include "application.hpp"
#include "errorHandler.hpp"
#include "pipeline.hpp"
#include "swapChain.hpp"
#include "types.hpp"
#include "vulkanApp.hpp"
#include "window.hpp"

void Application::init(){
    initWindow();
    initVulkan();
    initSwapChain();
    initPipelineLayout();
    initPipeline();
    initCommandBuffers();
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
    _Pipeline->cleanUp();
    vkDestroyPipelineLayout(_VulkanApp->getDevice(), _PipelineLayout, nullptr);
    _SwapChain->cleanUp();
    _VulkanApp->cleanUp();
    _Window->cleanUp();
}

void Application::drawFrame(){
}

void Application::run(){
    init();
    mainLoop();
    cleanUp();
}

void Application::initWindow(){
    _Window = WindowPtr(
        new Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello vulkan!")
    );
    _Window->init();
}

void Application::initPipeline(){
    _Pipeline = PipelinePtr(new Pipeline(_VulkanApp));
    _Pipeline->initVertexShader("shaders/compiled/basicTriangleVert.spv");
    _Pipeline->initFragmentShader("shaders/compiled/basicTriangleFrag.spv");
    auto pipelineConfig = Pipeline::defaultPipelineConfigInfo(
        _SwapChain->getWidth(), 
        _SwapChain->getHeight()
    );
    pipelineConfig._RenderPass = _SwapChain->getRenderPass();
    pipelineConfig._PipelineLayout = _PipelineLayout;
    _Pipeline->init(pipelineConfig);
}

void Application::initSwapChain(){
    _SwapChain = SwapChainPtr(
        new SwapChain(
            _VulkanApp,
            _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities)
        )
    );
    _SwapChain->init();
}

void Application::initPipelineLayout(){
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    VkResult result = vkCreatePipelineLayout(
        _VulkanApp->getDevice(), 
        &pipelineLayoutInfo, 
        nullptr, 
        &_PipelineLayout
    );
    ErrorHandler::vulkanError(result, "Failed to create pipeline layout!\n");
}

void Application::initCommandBuffers(){

}