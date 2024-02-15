# pragma once

#include "window.hpp"
#include <memory>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

#include "vulkanInstance.hpp"

namespace BE{

class VulkanApp;
using VulkanAppPtr = std::shared_ptr<VulkanApp>;


struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return 
            graphicsFamily.has_value() 
            && presentFamily.has_value();
    }
};

struct VulkanSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanApp{

    public:
        #ifdef NDEBUG
            static const bool _VulkanEnableValidationLayers = false;
        #else
            static const bool _VulkanEnableValidationLayers = true;
        #endif

        static const std::vector<const char*> _VulkanValidationLayers;
        static const std::vector<const char*> _VulkanDeviceExtensions;

    private:
        WindowPtr _Window = nullptr;
        VulkanInstancePtr _Instance = nullptr;

        VkPhysicalDevice _VulkanPhysicalDevice = VK_NULL_HANDLE;

        VkCommandPool _VulkanCommandPool;
        VkDevice _VulkanLogicalDevice;
        VkSurfaceKHR _VulkanSurface;
        VkQueue _VulkanGraphicsQueue;
        VkQueue _VulkanPresentQueue;
        VkPhysicalDeviceProperties _Properties;

    public:
        VulkanApp(){};

        // Not copyable or movable
        VulkanApp(const VulkanApp &) = delete;
        void operator=(const VulkanApp &) = delete;
        VulkanApp(VulkanApp &&) = delete;
        VulkanApp &operator=(VulkanApp &&) = delete;
        
        void init(WindowPtr window);
        void cleanUp();

    public:
    VkPhysicalDeviceProperties getProperties() const {return _Properties;}
        VkCommandPool getCommandPool() const {return _VulkanCommandPool;}
        VkDevice getDevice() const {return _VulkanLogicalDevice;}
        VkSurfaceKHR getSurface() const {return _VulkanSurface;}
        VkQueue getGraphicsQueue() const {return _VulkanGraphicsQueue;}
        VkQueue getPresentQueue() const {return _VulkanPresentQueue;}

        VulkanSwapChainSupportDetails getSwapChainSupport(){ 
            return querySwapChainSupport(_VulkanPhysicalDevice); 
        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        VulkanQueueFamilyIndices findPhysicalQueueFamilies(){ 
            return findQueueFamilies(_VulkanPhysicalDevice); 
        }

        VkFormat findSupportedFormat(
            const std::vector<VkFormat> &candidates, 
            VkImageTiling tiling, 
            VkFormatFeatureFlags features
        );


        // Buffer Helper Functions
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory
        );

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount
        );

        void createImageWithInfo(
            const VkImageCreateInfo &imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage &image,
            VkDeviceMemory &imageMemory
        );

    private:
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        int rateDevice(VkPhysicalDevice device);

        bool isDeviceSuitable(VkPhysicalDevice device);
        VulkanQueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        bool checkDeviceExtensionsSupport(VkPhysicalDevice device);
        VulkanSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

};

};