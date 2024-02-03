#pragma once

#include "types.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class SwapChain;
using SwapChainPtr = std::shared_ptr<SwapChain>;

class SwapChain{

    private:
        const int VULKAN_MAX_FRAMES_IN_FLIGHT = 2;

        VulkanAppPtr _VulkanApp = nullptr;

        VkFormat _SwapChainImageFormat;
        VkExtent2D _SwapChainExtent;
        std::vector<VkFramebuffer> _SwapChainFramebuffers;
        std::vector<VkImage> _SwapChainImages;
        std::vector<VkImageView> _SwapChainImageViews;
        VkSwapchainKHR _SwapChain;
        
        VkRenderPass _RenderPass;

        std::vector<VkImage> _DepthImages;
        std::vector<VkDeviceMemory> _DepthImageMemorys;
        std::vector<VkImageView> _DepthImageViews;

        VkExtent2D _WindowExtent;


        std::vector<VkSemaphore> _ImageAvailableSemaphores;
        std::vector<VkSemaphore> _RenderFinishedSemaphores;
        std::vector<VkFence> _InFlightFences;
        std::vector<VkFence> _ImagesInFlight;
        size_t _CurrentFrame = 0;

    public:
        SwapChain(VulkanAppPtr vulkanApp, VkExtent2D extent) 
            : _VulkanApp(vulkanApp), _WindowExtent(extent){

        }

        void init();
        void cleanUp();

        VkFramebuffer getFrameBuffer(int index) const { 
            return _SwapChainFramebuffers[index]; 
        }

        VkRenderPass getRenderPass() const { 
            return _RenderPass; 
        }
        
        VkImageView getImageView(int index) const { 
            return _SwapChainImageViews[index]; 
        }

        size_t getImageCount() const { 
            return _SwapChainImages.size(); 
        }

        VkFormat getSwapChainImageFormat() const { 
            return _SwapChainImageFormat; 
        }

        VkExtent2D getSwapChainExtent() const { 
            return _SwapChainExtent; 
        }

        uint32_t getWidth() const { 
            return _SwapChainExtent.width; 
        }

        uint32_t getHeight() const { 
            return _SwapChainExtent.height; 
        }

        float getExtentAspectRatio() const {
            return static_cast<float>(getWidth()) / static_cast<float>(getHeight());
        }

        VkFormat findDepthFormat();
        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    private:
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats
        );
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes
        );
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

};