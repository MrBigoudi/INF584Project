#pragma once

#include "errorHandler.hpp"
#include "types.hpp"
#include <map>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkanApp.hpp>
#include <cstdint>
#include <vector>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

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

        VkBuffer _VertexBuffer;
        VkDeviceMemory _VertexBufferMemory;
        uint32_t _VertexCount = 0;

        bool _HasIndexBuffer = false;
        VkBuffer _IndexBuffer;
        VkDeviceMemory _IndexBufferMemory;
        uint32_t _IndexCount = 0;

    public:
        Model(VulkanAppPtr vulkanApp, const VertexDataBuilder& dataBuilder) 
            : _VulkanApp(vulkanApp){
            createVertexBuffer(dataBuilder._Vertices);
            createIndexBuffer(dataBuilder._Indices);
        }

        Model(VulkanAppPtr vulkanApp, const std::string& filePath)
            : _VulkanApp(vulkanApp){
            // check extension type
            size_t dotPosition = filePath.find_last_of(".");
            std::string extension = "";
            if(dotPosition != std::string::npos 
                && dotPosition != filePath.length() - 1){
                extension = filePath.substr(dotPosition + 1);
            }
            if(extension.empty()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "Trying to load a model from a file without extension: " + filePath +"!\n"
                );
            }
            
            auto it = MODEL_EXTENSIONS_MAP.find(extension);
            if(it == MODEL_EXTENSIONS_MAP.end()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "Trying to load a model from a file with an unkown extension: " + filePath +"!\n"
                );
            }
            ModelExtension extensionFormat = it->second; 
            VertexDataBuilder builder{};
            switch(extensionFormat) {
                case OFF:
                    builder.loadOffModel(filePath);
                    break;
                case OBJ:
                    builder.loadObjModel(filePath);
                    break;
                default:
                    ErrorHandler::handle(
                        ErrorCode::SYSTEM_ERROR,
                        "Error creating a model from a file, this error shouldn't have occured!\n"
                    );
            }
            createVertexBuffer(builder._Vertices);
            createIndexBuffer(builder._Indices);
        }

        void bind(VkCommandBuffer commandBuffer){
            VkBuffer buffers[] = {_VertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
            if(_HasIndexBuffer){
                vkCmdBindIndexBuffer(commandBuffer, _IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
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
            vkDestroyBuffer(_VulkanApp->getDevice(), _VertexBuffer, nullptr);
            vkFreeMemory(_VulkanApp->getDevice(), _VertexBufferMemory, nullptr);
            if(_HasIndexBuffer){
                vkDestroyBuffer(_VulkanApp->getDevice(), _IndexBuffer, nullptr);
                vkFreeMemory(_VulkanApp->getDevice(), _IndexBufferMemory, nullptr);
            }
        }

    private:
        void createVertexBuffer(const std::vector<VertexData>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);
};