#pragma once

#include <map>
#include <vulkan/vulkan.hpp>
#include <cstdint>
#include <vector>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

#include "buffer.hpp"
#include "types.hpp"

class Model;
using ModelPtr = std::shared_ptr<Model>;

struct VertexData{
    glm::vec3 _Pos = {0.f,0.f,0.f};
    glm::vec4 _Col = {0.f,0.f,0.f,1.f};
    glm::vec3 _Norm = {0.f,0.f,0.f};
    glm::vec2 _Tex = {0.f,0.f};

    static const uint32_t NB_LAYOUTS = 4;
    static const std::vector<VkFormat> FORMATS;
    static const std::vector<size_t> OFFSETS;

    bool operator==(const VertexData& other) const {
        return _Pos == other._Pos 
            && _Col == other._Col 
            && _Norm == other._Norm 
            && _Tex == other._Tex;
    }

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
            attributeDescription[i].offset = OFFSETS[i];
        }
        return attributeDescription;
    }

};

struct VertexDataBuilder{
    std::vector<VertexData> _Vertices{};
    std::vector<uint32_t> _Indices{};

    void loadOffModel(const std::string& filePath);
    void loadObjModel(const std::string& filePath);
};

class Model{

    private:
        enum ModelExtension{
            OFF,
            OBJ,
        };

        static const std::map<std::string, ModelExtension> MODEL_EXTENSIONS_MAP;

    public:
        static const uint32_t MIN_VERTEX_COUNT = 3;

    private:
        VulkanAppPtr _VulkanApp = nullptr; 

        BufferPtr _VertexBuffer = nullptr;
        uint32_t _VertexCount = 0;

        bool _HasIndexBuffer = false;
        BufferPtr _IndexBuffer = nullptr;
        uint32_t _IndexCount = 0;

    public:
        Model(VulkanAppPtr vulkanApp, const VertexDataBuilder& dataBuilder) 
            : _VulkanApp(vulkanApp){
            createVertexBuffer(dataBuilder._Vertices);
            createIndexBuffer(dataBuilder._Indices);
        }

        Model(VulkanAppPtr vulkanApp, const std::string& filePath);

        void bind(VkCommandBuffer commandBuffer){
            VkBuffer buffers[] = {_VertexBuffer->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
            if(_HasIndexBuffer){
                vkCmdBindIndexBuffer(commandBuffer, _IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            }
        }

        void draw(VkCommandBuffer commandBuffer){
            if(_HasIndexBuffer){
                vkCmdDrawIndexed(commandBuffer, _IndexCount, 1, 0, 0, 0);
            } else {
                vkCmdDraw(commandBuffer, _VertexCount, 1, 0, 0);
            }
        }
        
        void cleanUp(){
            if(_VertexBuffer != nullptr){
                _VertexBuffer->cleanUp();
                _VertexBuffer = nullptr;
            }
            if(_IndexBuffer != nullptr){
                _IndexBuffer->cleanUp();
                _IndexBuffer = nullptr;
            }
        }

    private:
        void createVertexBuffer(const std::vector<VertexData>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);
};