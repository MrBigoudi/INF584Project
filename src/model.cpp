#include "model.hpp"
#include "errorHandler.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>


void Model::createVertexBuffer(const std::vector<VertexData>& vertices){
    _VertexCount = static_cast<uint32_t>(vertices.size());
    if(_VertexCount < MIN_VERTEX_COUNT){
        ErrorHandler::handle(ErrorCode::BAD_VALUE_ERROR, "Vertex count must be at least 3!\n");
    }

    VkDeviceSize bufferSize = sizeof(vertices[0]) * _VertexCount;
    _VulkanApp->createBuffer(
        bufferSize, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        _VertexBuffer, 
        _VertexBufferMemory
    );

    void* data;
    vkMapMemory(_VulkanApp->getDevice(), _VertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(_VulkanApp->getDevice(), _VertexBufferMemory);
}

const std::vector<VkFormat> VertexData::FORMATS = {
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32_SFLOAT
};

const std::vector<size_t> VertexData::SIZES = {
    0,
    sizeof(VertexData::_Pos),
    sizeof(VertexData::_Col),
    sizeof(VertexData::_Norm),
    sizeof(VertexData::_Tex)
};