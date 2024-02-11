#include "model.hpp"
#include "errorHandler.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyObjectLoader.hpp"

void Model::createVertexBuffer(const std::vector<VertexData>& vertices){
    _VertexCount = static_cast<uint32_t>(vertices.size());
    if(_VertexCount < MIN_VERTEX_COUNT){
        ErrorHandler::handle(ErrorCode::BAD_VALUE_ERROR, "Vertex count must be at least 3!\n");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkDeviceSize bufferSize = sizeof(vertices[0]) * _VertexCount;
    // buffer to staging
    _VulkanApp->createBuffer(
        bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, 
        stagingBufferMemory
    );

    void* data;
    vkMapMemory(_VulkanApp->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_VulkanApp->getDevice(), stagingBufferMemory);

    // staging to host
    _VulkanApp->createBuffer(
        bufferSize, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _VertexBuffer, 
        _VertexBufferMemory
    );
    _VulkanApp->copyBuffer(stagingBuffer, _VertexBuffer, bufferSize);
    vkDestroyBuffer(_VulkanApp->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(_VulkanApp->getDevice(), stagingBufferMemory, nullptr);
}

void Model::createIndexBuffer(const std::vector<uint32_t>& indices){
    _IndexCount = static_cast<uint32_t>(indices.size());
    if(_IndexCount == 0){
        _HasIndexBuffer = false;
        return;
    }
    _HasIndexBuffer = true;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkDeviceSize bufferSize = sizeof(uint32_t) * _IndexCount;
    // buffer to staging
    _VulkanApp->createBuffer(
        bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, 
        stagingBufferMemory
    );

    void* data;
    vkMapMemory(
        _VulkanApp->getDevice(), 
        stagingBufferMemory, 
        0, 
        bufferSize, 
        0, 
        &data
    );
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_VulkanApp->getDevice(), stagingBufferMemory);

    // staging to host
    _VulkanApp->createBuffer(
        bufferSize, 
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _IndexBuffer, 
        _IndexBufferMemory
    );
    _VulkanApp->copyBuffer(stagingBuffer, _IndexBuffer, bufferSize);
    vkDestroyBuffer(_VulkanApp->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(_VulkanApp->getDevice(), stagingBufferMemory, nullptr);
}

const std::vector<VkFormat> VertexData::FORMATS = {
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT
};

const std::vector<size_t> VertexData::OFFSETS = {
    offsetof(VertexData, _Pos),
    offsetof(VertexData, _Col),
    offsetof(VertexData, _Norm),
    offsetof(VertexData, _Tex),
};