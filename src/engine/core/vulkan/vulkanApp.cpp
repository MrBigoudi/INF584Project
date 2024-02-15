#include "vulkanApp.hpp"
#include "errorHandler.hpp"
#include "application.hpp"
#include "window.hpp"

#include <map>
#include <set>

namespace BE{

const std::vector<const char*> VulkanApp::_VulkanValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> VulkanApp::_VulkanDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void VulkanApp::init(WindowPtr window){
    fprintf(stdout, "Start initializing vulkan...\n");
    _Window = window;
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
    fprintf(stdout, "Vulkan initialized\n");
}

void VulkanApp::cleanUp(){
    vkDestroyCommandPool(_VulkanLogicalDevice, _VulkanCommandPool, nullptr);
    vkDestroyDevice(_VulkanLogicalDevice, nullptr);

    vkDestroySurfaceKHR(_Instance->getInstance(), _VulkanSurface, nullptr);
    _Instance->cleanUp();
}

void VulkanApp::createInstance() {
    _Instance = VulkanInstancePtr(new VulkanInstance());
    _Instance->init();
}

void VulkanApp::pickPhysicalDevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_Instance->getInstance(), &deviceCount, nullptr);
    if(deviceCount == 0){
        ErrorHandler::handle(ErrorCode::VULKAN_ERROR, "Failed to find GPUs with Vulkan support!\n");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_Instance->getInstance(), &deviceCount, devices.data());
    
    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;
    for(const auto& device : devices){
        int score = rateDevice(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0) {
        _VulkanPhysicalDevice = candidates.rbegin()->second;
    } else {
        ErrorHandler::handle(ErrorCode::VULKAN_ERROR, "Failed to find a suitable GPU!\n");
    }
}

int VulkanApp::rateDevice(VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties); 
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    VulkanQueueFamilyIndices indices = findQueueFamilies(device);

    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    bool extensionsSupported = checkDeviceExtensionsSupport(device);
    bool swapChainAdequate = false;

    if(extensionsSupported){
        VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // Application can't function without geometry shaders and need
    if (!deviceFeatures.geometryShader 
        || !indices.isComplete()
        || !extensionsSupported
        || !swapChainAdequate
        ) {
        return 0;
    }

    return score;
}

VulkanQueueFamilyIndices VulkanApp::findQueueFamilies(VkPhysicalDevice device){
    VulkanQueueFamilyIndices indices;
    // Logic to find graphics queue family
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device, 
            i, 
            _VulkanSurface, 
            &presentSupport
        );
        if(presentSupport){
            indices.presentFamily = i;
        }

        if(indices.isComplete()){
            break;
        }

        i++;
    }

    return indices;
}

void VulkanApp::createLogicalDevice(){
    VulkanQueueFamilyIndices indices = findQueueFamilies(_VulkanPhysicalDevice);
   
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(), 
        indices.presentFamily.value()
    };
    
    float queuePriority = 1.f;
    for(uint32_t queueFamily : uniqueQueueFamilies){
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(_VulkanDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = _VulkanDeviceExtensions.data();

    if (_VulkanEnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_VulkanValidationLayers.size());
        createInfo.ppEnabledLayerNames = _VulkanValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(_VulkanPhysicalDevice, &createInfo, nullptr, &_VulkanLogicalDevice) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device!\n");
        exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(_VulkanLogicalDevice, indices.graphicsFamily.value(), 0, &_VulkanGraphicsQueue);
    vkGetDeviceQueue(_VulkanLogicalDevice, indices.presentFamily.value(), 0, &_VulkanPresentQueue);
}

void VulkanApp::createSurface(){
    VkResult result = _Window->createWindowSurface(_Instance->getInstance(), _VulkanSurface);
    ErrorHandler::vulkanError(result, "Failed to create a window surface!\n");
}

void VulkanApp::createCommandPool(){
    VulkanQueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkResult result = vkCreateCommandPool(
        _VulkanLogicalDevice, 
        &poolInfo, 
        nullptr, 
        &_VulkanCommandPool
    );
    
    ErrorHandler::vulkanError(result, "Failed to create command pool!\n");
}

bool VulkanApp::checkDeviceExtensionsSupport(VkPhysicalDevice device){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(_VulkanDeviceExtensions.begin(), _VulkanDeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VulkanSwapChainSupportDetails VulkanApp::querySwapChainSupport(VkPhysicalDevice device){
    VulkanSwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, 
        _VulkanSurface, 
        &details.capabilities
    );
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, 
        _VulkanSurface, 
        &formatCount, 
        nullptr
    );
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, 
            _VulkanSurface, 
            &formatCount, 
            details.formats.data()
        );
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, 
        _VulkanSurface, 
        &presentModeCount, 
        nullptr
    );
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, 
            _VulkanSurface, 
            &presentModeCount, 
            details.presentModes.data()
        );
    }

    return details;
}

VkFormat VulkanApp::findSupportedFormat(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_VulkanPhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (
            tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    ErrorHandler::handle(ErrorCode::VULKAN_ERROR, "Failed to find supported format!\n");
    return candidates[0];
}

uint32_t VulkanApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_VulkanPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    ErrorHandler::handle(ErrorCode::VULKAN_ERROR, "Failed to find suitable memory type!\n");
    return 0;
}

void VulkanApp::createBuffer(
        VkDeviceSize size, 
        VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer, 
        VkDeviceMemory& bufferMemory
        )
    {

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(_VulkanLogicalDevice, &bufferInfo, nullptr, &buffer);
    ErrorHandler::vulkanError(result, "Failed to create vertex buffer!\n");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_VulkanLogicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(_VulkanLogicalDevice, &allocInfo, nullptr, &bufferMemory); 
    ErrorHandler::vulkanError(result, "Failed to allocate vertex buffer memory!\n");

    vkBindBufferMemory(_VulkanLogicalDevice, buffer, bufferMemory, 0);
}


VkCommandBuffer VulkanApp::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _VulkanCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_VulkanLogicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void VulkanApp::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_VulkanGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_VulkanGraphicsQueue);

    vkFreeCommandBuffers(_VulkanLogicalDevice, _VulkanCommandPool, 1, &commandBuffer);
}


void VulkanApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}


void VulkanApp::copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    endSingleTimeCommands(commandBuffer);
}


void VulkanApp::createImageWithInfo(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory) {

    VkResult result = vkCreateImage(_VulkanLogicalDevice, &imageInfo, nullptr, &image);
    ErrorHandler::vulkanError(result, "Failed to create image!\n");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_VulkanLogicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(_VulkanLogicalDevice, &allocInfo, nullptr, &imageMemory);
    ErrorHandler::vulkanError(result, "Failed to allocate image memory!\n");

    result = vkBindImageMemory(_VulkanLogicalDevice, image, imageMemory, 0);
    ErrorHandler::vulkanError(result, "Failed to bind image memory!\n");
}

};