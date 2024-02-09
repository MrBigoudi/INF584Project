#include "swapChain.hpp"
#include "errorHandler.hpp"
#include "vulkanApp.hpp"
#include <cstdint>

void SwapChain::init(){
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

void SwapChain::cleanUp(){
    if(_OldSwapChain != nullptr){
        _OldSwapChain->cleanUp();
    }

    for (auto imageView : _SwapChainImageViews) {
        vkDestroyImageView(_VulkanApp->getDevice(), imageView, nullptr);
    }
    _SwapChainImageViews.clear();

    if (_SwapChain != nullptr) {
        vkDestroySwapchainKHR(_VulkanApp->getDevice(), _SwapChain, nullptr);
        _SwapChain = nullptr;
    }

    for (int i = 0; i < _DepthImages.size(); i++) {
        vkDestroyImageView(_VulkanApp->getDevice(), _DepthImageViews[i], nullptr);
        vkDestroyImage(_VulkanApp->getDevice(), _DepthImages[i], nullptr);
        vkFreeMemory(_VulkanApp->getDevice(), _DepthImageMemorys[i], nullptr);
    }

    for (auto framebuffer : _SwapChainFramebuffers) {
        vkDestroyFramebuffer(_VulkanApp->getDevice(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(_VulkanApp->getDevice(), _RenderPass, nullptr);

    // cleanup synchronization objects
    for (size_t i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(_VulkanApp->getDevice(), _RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_VulkanApp->getDevice(), _ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_VulkanApp->getDevice(), _InFlightFences[i], nullptr);
    }
}

VkResult SwapChain::acquireNextImage(uint32_t *imageIndex) {
    vkWaitForFences(
        _VulkanApp->getDevice(),
        1,
        &_InFlightFences[_CurrentFrame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max()
    );

    VkResult result = vkAcquireNextImageKHR(
        _VulkanApp->getDevice(),
        _SwapChain,
        std::numeric_limits<uint64_t>::max(),
        _ImageAvailableSemaphores[_CurrentFrame],  // must be a not signaled semaphore
        VK_NULL_HANDLE,
        imageIndex
    );

    return result;
}

VkResult SwapChain::submitCommandBuffers(
    const VkCommandBuffer *buffers, uint32_t *imageIndex) {
    if (_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(
            _VulkanApp->getDevice(), 
            1, 
            &_ImagesInFlight[*imageIndex], 
            VK_TRUE, 
            UINT64_MAX
        );
    }
    _ImagesInFlight[*imageIndex] = _InFlightFences[_CurrentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_ImageAvailableSemaphores[_CurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {_RenderFinishedSemaphores[_CurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_VulkanApp->getDevice(), 1, &_InFlightFences[_CurrentFrame]);
    VkResult result = vkQueueSubmit(
        _VulkanApp->getGraphicsQueue(), 
        1, 
        &submitInfo, 
        _InFlightFences[_CurrentFrame]
    );
    ErrorHandler::vulkanError(result, "Failed to submit draw command buffer!\n");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_SwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    result = vkQueuePresentKHR(_VulkanApp->getPresentQueue(), &presentInfo);

    _CurrentFrame = (_CurrentFrame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;

    return result;
}

void SwapChain::createSwapChain() {
    VulkanSwapChainSupportDetails swapChainSupport = _VulkanApp->getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _VulkanApp->getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VulkanQueueFamilyIndices indices = _VulkanApp->findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(), 
        indices.presentFamily.value()
    };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = _OldSwapChain == nullptr 
        ? VK_NULL_HANDLE
        : _OldSwapChain->_SwapChain
    ;

    VkResult result = vkCreateSwapchainKHR(_VulkanApp->getDevice(), &createInfo, nullptr, &_SwapChain);
    ErrorHandler::vulkanError(result, "Failed to create swap chain!\n");

    vkGetSwapchainImagesKHR(_VulkanApp->getDevice(), _SwapChain, &imageCount, nullptr);
    _SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_VulkanApp->getDevice(), _SwapChain, &imageCount, _SwapChainImages.data());

    _SwapChainImageFormat = surfaceFormat.format;
    _SwapChainExtent = extent;
}

void SwapChain::createImageViews() {
    _SwapChainImageViews.resize(_SwapChainImages.size());
    for (size_t i = 0; i < _SwapChainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _SwapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = _SwapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(_VulkanApp->getDevice(), &viewInfo, nullptr, &_SwapChainImageViews[i]);
        ErrorHandler::vulkanError(result, "Failed to create texture image view!\n");
    }
}

void SwapChain::createRenderPass() {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = getSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(_VulkanApp->getDevice(), &renderPassInfo, nullptr, &_RenderPass);
    ErrorHandler::vulkanError(result, "Failed to create render pass!\n");
}

void SwapChain::createFramebuffers() {
    const uint32_t imageCount = getImageCount();
    _SwapChainFramebuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++) {
        std::array<VkImageView, 2> attachments = {
            _SwapChainImageViews[i], 
            _DepthImageViews[i]
        };

        VkExtent2D swapChainExtent = getSwapChainExtent();
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(
                _VulkanApp->getDevice(),
                &framebufferInfo,
                nullptr,
                &_SwapChainFramebuffers[i]
        );
        ErrorHandler::vulkanError(result, "Failed to create framebuffer!\n");
    }
}

void SwapChain::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    _SwapChainDepthFormat = depthFormat;
    VkExtent2D swapChainExtent = getSwapChainExtent();

    const uint32_t imageCount = getImageCount();
    _DepthImages.resize(imageCount);
    _DepthImageMemorys.resize(imageCount);
    _DepthImageViews.resize(imageCount);

    for (int i = 0; i < imageCount; i++) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        _VulkanApp->createImageWithInfo(
            imageInfo,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _DepthImages[i],
            _DepthImageMemorys[i]
        );

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _DepthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(
            _VulkanApp->getDevice(), 
            &viewInfo, 
            nullptr, 
            &_DepthImageViews[i]
        );
        ErrorHandler::vulkanError(result, "Failed to create texture image view!\n");
    }
}

void SwapChain::createSyncObjects() {
    _ImageAvailableSemaphores.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
    _RenderFinishedSemaphores.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
    _InFlightFences.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
    _ImagesInFlight.resize(getImageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; i++) {
        VkResult result;
        result = vkCreateSemaphore(
                    _VulkanApp->getDevice(), 
                    &semaphoreInfo, 
                    nullptr, 
                    &_ImageAvailableSemaphores[i]
        );
        ErrorHandler::vulkanError(result, "Failed to create `image available` semaphore!\n");

        result = vkCreateSemaphore(
                    _VulkanApp->getDevice(), 
                    &semaphoreInfo, 
                    nullptr, 
                    &_RenderFinishedSemaphores[i]
        );
        ErrorHandler::vulkanError(result, "Failed to create `render finished` semaphore!\n");
         
        result = vkCreateFence(
            _VulkanApp->getDevice(), 
            &fenceInfo, 
            nullptr, 
            &_InFlightFences[i]
        );
        ErrorHandler::vulkanError(result, "Failed to create `in flight` fence!\n");
    }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
    // for (const auto &availablePresentMode : availablePresentModes) {
    //     if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
    //         fprintf(stdout, "Present mode: Mailbox\n");
    //         return availablePresentMode;
    //     }
    // }

    // for (const auto &availablePresentMode : availablePresentModes) {
    //    if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //        fprintf(stdout, "Present mode: Immediate\n");
    //        return availablePresentMode;
    //    }
    // }

    fprintf(stdout, "Present mode: V-Sync\n");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = _WindowExtent;
        actualExtent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkFormat SwapChain::findDepthFormat() {
    auto candidates = {
        VK_FORMAT_D32_SFLOAT, 
        VK_FORMAT_D32_SFLOAT_S8_UINT, 
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    return _VulkanApp->findSupportedFormat(
        candidates,
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool SwapChain::compareSwapFormat(const SwapChainPtr swapChain) const {
    return swapChain->_SwapChainDepthFormat == _SwapChainDepthFormat &&
           swapChain->_SwapChainImageFormat == _SwapChainImageFormat;
}