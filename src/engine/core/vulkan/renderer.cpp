#include "renderer.hpp"

#include "errorHandler.hpp"
#include "swapChain.hpp"
#include "vulkanApp.hpp"

namespace BE{


Renderer::Renderer(WindowPtr window, VulkanAppPtr vulkanApp)
    : _Window(window), _VulkanApp(vulkanApp){
    recreateSwapChain();
    initCommandBuffers();
}

void Renderer::cleanUpCommandBuffers(){
    if(!_CommandBuffers.empty()){
        vkFreeCommandBuffers(_VulkanApp->getDevice(), 
            _VulkanApp->getCommandPool(),
            static_cast<uint32_t>(_CommandBuffers.size()), 
            _CommandBuffers.data()
        );
        _CommandBuffers.clear();
    }
}

void Renderer::cleanUp(){
    cleanUpCommandBuffers();
    _SwapChain->cleanUp();
}

void Renderer::initSwapChain(){
    if(_SwapChain == nullptr){
        _SwapChain = SwapChainPtr(
            new SwapChain(
                _VulkanApp,
                _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities)
            )
        );
    } else {
        SwapChainPtr oldSwapChain = _SwapChain;
        _SwapChain = SwapChainPtr(
            new SwapChain(
                _VulkanApp,
                _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities),
                oldSwapChain
            )
        );

        if(!oldSwapChain->compareSwapFormat(_SwapChain)){
            ErrorHandler::handle(
                ErrorCode::UNEXPECTED_VALUE_ERROR,
                "Swap chain image or depth format has changed!\n"
            );
        }
    }
}


void Renderer::initCommandBuffers(){
    _CommandBuffers.resize(SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = _VulkanApp->getCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint32_t>(_CommandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(_VulkanApp->getDevice(), &allocateInfo, _CommandBuffers.data());
    ErrorHandler::vulkanError(result, "Failed to allocate the command buffers!\n");
}

void Renderer::recreateSwapChain(){
    auto extent = _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities);
    while(extent.width == 0 || extent.height == 0){
        extent = _Window->getExtent(_VulkanApp->getSwapChainSupport().capabilities);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(_VulkanApp->getDevice());

    initSwapChain();
}

VkCommandBuffer Renderer::beginFrame(){
    if(_IsFrameStarted){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Can't call begin frame while already in progress!\n"
        );
    }

    VkResult result = _SwapChain->acquireNextImage(&_CurrentImageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return nullptr;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        ErrorHandler::handle(
            ErrorCode::VULKAN_ERROR, 
            "Failed to acquire next swap chain image!\n"
        );
    }

    _IsFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    ErrorHandler::vulkanError(result, "Failed to begin recording command buffer!\n");

    return commandBuffer;
}

void Renderer::endFrame(){
    if(!_IsFrameStarted){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Can't call end frame if frame is not in progress"
        );
    }
    
    auto commandBuffer = getCurrentCommandBuffer();
    VkResult result = vkEndCommandBuffer(commandBuffer);
    ErrorHandler::vulkanError(result, "Failed to record command buffer!\n");

    result = _SwapChain->submitCommandBuffers(&commandBuffer, &_CurrentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR 
        || result == VK_SUBOPTIMAL_KHR
        || _Window->wasWindowResized()
    ){
        _Window->resetWindowResizedFlag();
        recreateSwapChain();
    } else {
        ErrorHandler::vulkanError(result, "Failed to present swap chain image!\n");
    }

    _IsFrameStarted = false;
    _CurrentFrameIndex = (_CurrentFrameIndex + 1) % SwapChain::VULKAN_MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
    if(!_IsFrameStarted){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Can't begin render pass if frame is not in progress!\n"
        );
    }

    if(commandBuffer != getCurrentCommandBuffer()){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Can't begin render pass on command buffer from a different frame!\n"
        );
    }

    // begin render pass command
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _SwapChain->getRenderPass();
    renderPassInfo.framebuffer = _SwapChain->getFrameBuffer(_CurrentImageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _SwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    // in pipeline, 0 is the color attachement, 1 is the depths
    clearValues[0].color = {0.383f, 0.632f, 0.800f, 1.f};
    clearValues[1].depthStencil = {1.f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_SwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(_SwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _SwapChain->getSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
    if(!_IsFrameStarted){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Can't end render pass if frame is not in progress!\n"
        );
    }

    if(commandBuffer != getCurrentCommandBuffer()){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Can't end render pass on command buffer from a different frame!\n"
        );
    }

    vkCmdEndRenderPass(commandBuffer);
}

};