# pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <GLFW/glfw3.h>

#include "types.hpp"

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

        ApplicationPtr _Application = nullptr;

    public:
        VulkanApp();
        void init(ApplicationPtr app);
        void cleanUp();
};