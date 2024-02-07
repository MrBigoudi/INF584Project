#include <cstdint>
#include <iostream>
#include <array>
#include <vulkan/vulkan_core.h>

#include "window.hpp"
#include "application.hpp"
#include "errorHandler.hpp"
#include "model.hpp"
#include "pipeline.hpp"
#include "swapChain.hpp"
#include "types.hpp"
#include "vulkanApp.hpp"

void Application::init(){
    initWindow();
    initVulkan();
    // initSwapChain();
    initModels();
    initPipelineLayout();
    recreateSwapChain();
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

void Application::cleanUpCommandBuffers(){
    if(!_CommandBuffers.empty()){
        vkFreeCommandBuffers(_VulkanApp->getDevice(), 
            _VulkanApp->getCommandPool(),
            static_cast<uint32_t>(_CommandBuffers.size()), 
            _CommandBuffers.data()
        );
        _CommandBuffers.clear();
    }
}

void Application::cleanUp(){
    cleanUpCommandBuffers();
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

    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        ErrorHandler::handle(ErrorCode::VULKAN_ERROR, "Failed to acquire next swap chain image!\n");
    }

    recordCommandBuffer(imageIndex);
    result = _SwapChain->submitCommandBuffers(&_CommandBuffers[imageIndex], &imageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR 
        || result == VK_SUBOPTIMAL_KHR
        || _Window->wasWindowResized()
    ){
        _Window->resetWindowResizedFlag();
        recreateSwapChain();
        return;
    }
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
    if(_Pipeline != nullptr){
        _Pipeline->cleanUp();
    }

    if(_SwapChain == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a swap chain!\n"
        );
    }

    if(_PipelineLayout == nullptr){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR, 
            "Can't create a pipeline without a pipeline layout!\n"
        );
    }

    _Pipeline = PipelinePtr(new Pipeline(_VulkanApp));
    _Pipeline->initVertexShader("shaders/compiled/basicTriangleVert.spv");
    _Pipeline->initFragmentShader("shaders/compiled/basicTriangleFrag.spv");
    auto pipelineConfig = Pipeline::defaultPipelineConfigInfo();
    pipelineConfig._RenderPass = _SwapChain->getRenderPass();
    pipelineConfig._PipelineLayout = _PipelineLayout;
    _Pipeline->init(pipelineConfig);
}

void Application::initSwapChain(){
    if(_SwapChain == nullptr){
        _SwapChain = SwapChainPtr(
            new SwapChain(
                _VulkanApp,
                _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities)
            )
        );
    } else {
        _SwapChain = SwapChainPtr(
            new SwapChain(
                _VulkanApp,
                _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities),
                _SwapChain
            )
        );
    }
    _SwapChain->init();
}

void Application::initPipelineLayout(){

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = 
        VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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
}

void Application::initModels(){
    std::vector<VertexData> vertices{
        {{0.f, -0.5f, 0.f}, {1.f, 0.f, 0.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{0.5f, 0.5f, 0.f}, {0.f, 1.f, 0.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}},
        {{-0.5f, 0.5f, 0.f}, {0.f, 0.f, 1.f, 1.f}, {0.f,0.f,0.f}, {0.f,0.f}}
    };

    _Model = ModelPtr(new Model(_VulkanApp, vertices));
}

void Application::recreateSwapChain(){
    auto extent = _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities);
    while(extent.width == 0 || extent.height == 0){
        extent = _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(_VulkanApp->getDevice());

    initSwapChain();
    if(_SwapChain->getImageCount() != _CommandBuffers.size()){
        cleanUpCommandBuffers();
        initCommandBuffers();
    }

    initPipeline();
}

void Application::recordCommandBuffer(int imageIndex){
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    VkResult result = vkBeginCommandBuffer(_CommandBuffers[imageIndex], &beginInfo);
    ErrorHandler::vulkanError(result, "Failed to begin recording command buffer!\n");

    // begin render pass command
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _SwapChain->getRenderPass();
    renderPassInfo.framebuffer = _SwapChain->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _SwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    // in pipeline, 0 is the color attachement, 1 is the depths
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.f};
    clearValues[1].depthStencil = {1.f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_SwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(_SwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_CommandBuffers[imageIndex], 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _SwapChain->getSwapChainExtent();
    vkCmdSetScissor(_CommandBuffers[imageIndex], 0, 1, &scissor);
    
    _Pipeline->bind(_CommandBuffers[imageIndex]);
    _Model->bind(_CommandBuffers[imageIndex]);

    SimplePushConstantData push{};
    push._Random = static_cast<float>(glfwGetTime());
    vkCmdPushConstants(
        _CommandBuffers[imageIndex], 
        _PipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
        0, 
        sizeof(SimplePushConstantData), 
        &push
    );

    _Model->draw(_CommandBuffers[imageIndex]);

    vkCmdEndRenderPass(_CommandBuffers[imageIndex]);
    result = vkEndCommandBuffer(_CommandBuffers[imageIndex]);
    ErrorHandler::vulkanError(result, "Failed to record command buffer!\n");
}