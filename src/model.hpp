#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkanApp.hpp>
#include <cstdint>
#include <vector>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

class Model;
using ModelPtr = std::shared_ptr<Model>;

struct VertexData{
    glm::vec3 _Pos;
    glm::vec4 _Col;
    glm::vec3 _Norm;
    glm::vec2 _Tex;

    static const uint32_t NB_LAYOUTS = 4;
    static const std::vector<VkFormat> FORMATS;
    static const std::vector<size_t> SIZES;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(){
        std::vector<VkVertexInputBindingDescription> bindingDescription(1);
        bindingDescription[0].binding = 0;
        bindingDescription[0].stride = sizeof(VertexData);
        bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> attributeDescription(NB_LAYOUTS);
        for(int i=0; i<NB_LAYOUTS; i++){
            attributeDescription[i].binding = 0;
            attributeDescription[i].location = i;
            attributeDescription[i].format = FORMATS[i];
            attributeDescription[i].offset = SIZES[i];
        }
        return attributeDescription;
    }

};

class Model{

    public:
        static const uint32_t MIN_VERTEX_COUNT = 3;

    private:
        VulkanAppPtr _VulkanApp = nullptr; 
        VkBuffer _VertexBuffer;
        VkDeviceMemory _VertexBufferMemory;
        uint32_t _VertexCount = 0;

    public:
        Model(VulkanAppPtr vulkanApp, const std::vector<VertexData>& vertices) 
            : _VulkanApp(vulkanApp){
            createVertexBuffer(vertices);
        }

        void bind(VkCommandBuffer commandBuffer){
            VkBuffer buffers[] = {_VertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        }

        void draw(VkCommandBuffer commandBuffer){
            vkCmdDraw(commandBuffer, _VertexCount, 1, 0, 0);
        }
        
        void cleanUp(){
            vkDestroyBuffer(_VulkanApp->getDevice(), _VertexBuffer, nullptr);
            vkFreeMemory(_VulkanApp->getDevice(), _VertexBufferMemory, nullptr);
        }

    private:
        void createVertexBuffer(const std::vector<VertexData>& vertices);
};