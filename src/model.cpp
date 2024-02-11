#include "model.hpp"
#include "errorHandler.hpp"
#include <cstdint>
#include <unordered_map>


#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyObjectLoader.hpp"

#include "utils.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

template<>
struct std::hash<VertexData>{
    size_t operator()(const VertexData& vert) const {
        size_t seed = 0;
        hashCombine(seed, vert._Pos, vert._Col, vert._Norm, vert._Tex);
        return seed;
    }
};


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

void VertexDataBuilder::loadObjModel(const std::string& filePath){
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string error;

    auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filePath.c_str());
    if(!result){
        ErrorHandler::handle(
            ErrorCode::TINYOBJ_ERROR,
            "Failed to load .obj model: " + filePath + "!\n"
        );
    }

    _Vertices.clear();
    _Indices.clear();

    std::unordered_map<VertexData, uint32_t> uniqueVertices{};

    for(const auto& shape : shapes){
        for(const auto& index : shape.mesh.indices){
            VertexData vertex{};

            if(index.vertex_index >= 0){
                vertex._Pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                auto colorIndex = 3 * index.vertex_index + 2;
                if(colorIndex < attrib.colors.size()){
                    vertex._Col = {
                        attrib.vertices[colorIndex - 2],
                        attrib.vertices[colorIndex - 1],
                        attrib.vertices[colorIndex - 0],
                        1.f
                    };
                } else {
                    // default color
                    vertex._Col = {0.f, 0.f, 0.f, 1.f};
                }
            }

            if(index.normal_index >= 0){
                vertex._Norm = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if(index.texcoord_index >= 0){
                vertex._Tex = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            if(uniqueVertices.count(vertex) == 0){
                uniqueVertices[vertex] = static_cast<uint32_t>(_Vertices.size());
                _Vertices.push_back(vertex);
            }

            _Indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void VertexDataBuilder::loadOffModel(const std::string& filePath){
    fprintf(stdout, "Loading model: %s ...\n", filePath.c_str());
	std::ifstream file(filePath.c_str());
    if(!file){
        ErrorHandler::handle(
            ErrorCode::IO_ERROR,
            "Failed to load off model: " + filePath + "!\n"
        );
    }
	
    std::string line;
    std::getline(file, line);
    if(line.find("OFF") == std::string::npos){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Trying to load a non .off file: " + filePath + "!\n"
        );
    }

    std::getline(file, line);

    std::istringstream iss(line);
    int numVertices, numFaces, numCells;
    iss >> numVertices >> numFaces >> numCells;

    _Vertices.clear();
    _Indices.clear();

    // vertices
    for(int i = 0; i < numVertices; i++){
        VertexData vertex = {};
        std::getline(file, line);
        std::istringstream iss(line);
        float x, y, z;
        iss >> x >> y >> z;
        vertex._Pos = {x, y, z};
        _Vertices.push_back(vertex);
    }

    // indices
    for(int i=0; i<numFaces; i++){
        int numIndicesPerFace = 0;
        file >> numIndicesPerFace;
        for(int j=0; j<numIndicesPerFace; j++){
            int idx = 0;
            file >> idx;
            _Indices.push_back(idx);
        }
    }

    file.close();
    fprintf(stdout, "Done loading the model!\n");
}

const std::map<std::string, Model::ModelExtension> Model::MODEL_EXTENSIONS_MAP = {
    {"off", OFF},
    {"obj", OBJ},
};