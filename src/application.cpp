#include <cstdint>
#include <iostream>
#include <array>
#include <vulkan/vulkan_core.h>

#include "application.hpp"
#include "errorHandler.hpp"
#include "model.hpp"
#include "pipeline.hpp"
#include "swapChain.hpp"
#include "types.hpp"
#include "vulkanApp.hpp"
#include "window.hpp"

void Application::init(){
    initWindow();
    initVulkan();
    initSwapChain();
    initModels();
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
    vkDeviceWaitIdle(_VulkanApp->getDevice());
}

void Application::cleanUp(){
    _Model->cleanUp();
    _Pipeline->cleanUp();
    vkDestroyPipelineLayout(_VulkanApp->getDevice(), _PipelineLayout, nullptr);
    
    _SwapChain->cleanUp();
    _VulkanApp->cleanUp();
    _Window->cleanUp();
}

void Application::drawFrame(){
    uint32_t imageIndex = 0;
    VkResult result = _SwapChain->acquireNextImage(&imageIndex);
    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        ErrorHandler::handle(ErrorCode::VULKAN_ERROR, "Failed to acquire next swap chain image!\n");
    }

    result = _SwapChain->submitCommandBuffers(&_CommandBuffers[imageIndex], &imageIndex);
    ErrorHandler::vulkanError(result, "Failed to present swap chain image!\n");


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
    _CommandBuffers.resize(_SwapChain->getImageCount());
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = _VulkanApp->getCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint32_t>(_CommandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(_VulkanApp->getDevice(), &allocateInfo, _CommandBuffers.data());
    ErrorHandler::vulkanError(result, "Failed to allocate the command buffers!\n");

    for(int i=0; i<_CommandBuffers.size(); i++){
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        result = vkBeginCommandBuffer(_CommandBuffers[i], &beginInfo);
        ErrorHandler::vulkanError(result, "Failed to begin recording command buffer!\n");

        // begin render pass command
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _SwapChain->getRenderPass();
        renderPassInfo.framebuffer = _SwapChain->getFrameBuffer(i);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _SwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        // in pipeline, 0 is the color attachement, 1 is the depths
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.f};
        clearValues[1].depthStencil = {1.f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        _Pipeline->bind(_CommandBuffers[i]);
        // vkCmdDraw(_CommandBuffers[i], 3, 1, 0, 0);
        _Model->bind(_CommandBuffers[i]);
        _Model->draw(_CommandBuffers[i]);

        vkCmdEndRenderPass(_CommandBuffers[i]);
        result = vkEndCommandBuffer(_CommandBuffers[i]);
        ErrorHandler::vulkanError(result, "Failed to record command buffer!\n");
    }

}

void Application::initModels(){
    std::vector<VertexData> vertices{
        {{0.f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{0.5f, 0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}}
    };

    _Model = ModelPtr(new Model(_VulkanApp, vertices));
}