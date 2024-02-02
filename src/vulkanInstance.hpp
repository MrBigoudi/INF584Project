#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>

class VulkanInstance;
using VulkanInstancePtr = std::shared_ptr<VulkanInstance>;

class VulkanInstance{

    private:        
        VkInstance _Instance;
        VkDebugUtilsMessengerEXT _DebugMessenger;

    private:
        std::vector<const char*> vulkanGetRequiredExtensions();
        std::vector<VkExtensionProperties> vulkanGetSupportedExtensions(bool displayInfo = false);
        std::vector<VkLayerProperties> vulkanGetAvailableLayers(bool displayInfo = false);
        void vulkanCheckValidationLayerSupport(const std::vector<VkLayerProperties>& availableLayers);
        void vulkanPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void vulkanSetupDebugMessenger();

        void vulkanCheckGlfwExtensions(
            const std::vector<VkExtensionProperties>& supportedExtensions, 
            const char** glfwExtensions, uint32_t nbGlfwExtensions
        );
        
        VkResult vulkanCreateDebugUtilsMessengerEXT(
            VkInstance instance, 
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
            const VkAllocationCallbacks* pAllocator, 
            VkDebugUtilsMessengerEXT* pDebugMessenger
        );

        static void vulkanDestroyDebugUtilsMessengerEXT(
            VkInstance instance, 
            VkDebugUtilsMessengerEXT debugMessenger, 
            const VkAllocationCallbacks* pAllocator
        );

        static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );

    public:
        VulkanInstance(){};
        void init();
        void cleanUp();
        VkInstance getInstance() const;

};