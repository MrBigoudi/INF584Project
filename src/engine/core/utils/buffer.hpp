#pragma once

#include "vulkanApp.hpp"

#include <vulkan/vulkan.hpp>

#include <memory>

namespace BE{

class Buffer;
using BufferPtr = std::shared_ptr<Buffer>;

class Buffer{

    private:
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
        
        VulkanAppPtr _VulkanApp = nullptr;
        void* _Mapped = nullptr;
        VkBuffer _Buffer = VK_NULL_HANDLE;
        VkDeviceMemory _Memory = VK_NULL_HANDLE;
        
        VkDeviceSize _BufferSize = 0;
        uint32_t _InstanceCount = 0;
        VkDeviceSize _InstanceSize = 0;
        VkDeviceSize _AlignmentSize = 0;
        VkBufferUsageFlags _UsageFlags = 0;
        VkMemoryPropertyFlags _MemoryPropertyFlags = 0;

    public:
        Buffer(
            VulkanAppPtr vulkanApp,
            VkDeviceSize instanceSize,
            uint32_t instanceCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment = 1);

        void cleanUp();
        
        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();
        
        void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        
        void writeToIndex(void* data, int index);
        VkResult flushIndex(int index);
        VkDescriptorBufferInfo descriptorInfoForIndex(int index);
        VkResult invalidateIndex(int index);
        
        VkBuffer getBuffer() const { return _Buffer; }
        void* getMappedMemory() const { return _Mapped; }
        uint32_t getInstanceCount() const { return _InstanceCount; }
        VkDeviceSize getInstanceSize() const { return _InstanceSize; }
        VkDeviceSize getAlignmentSize() const { return _AlignmentSize; }
        VkBufferUsageFlags getUsageFlags() const { return _UsageFlags; }
        VkMemoryPropertyFlags getMemoryPropertyFlags() const { return _MemoryPropertyFlags; }
        VkDeviceSize getBufferSize() const { return _BufferSize; }

};

};