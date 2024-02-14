#include "model.hpp"
#include "buffer.hpp"
#include "errorHandler.hpp"
#include <cstdint>
#include <unordered_map>


#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyObjectLoader.hpp"

#include "utils.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "vulkanApp.hpp"


template<>
struct std::hash<BE::VertexData>{
    size_t operator()(const BE::VertexData& vert) const {
        size_t seed = 0;
        BE::hashCombine(seed, vert._Pos, vert._Col, vert._Norm, vert._Tex);
        return seed;
    }
};

namespace BE{

void Model::createVertexBuffer(const std::vector<VertexData>& vertices){
    _VertexCount = static_cast<uint32_t>(vertices.size());
    if(_VertexCount < MIN_VERTEX_COUNT){
        ErrorHandler::handle(ErrorCode::BAD_VALUE_ERROR, "Vertex count must be at least 3!\n");
    }

    uint32_t vertexSize = sizeof(vertices[0]);

    // buffer to staging
    Buffer stagingBuffer{
        _VulkanApp,
        vertexSize,
        _VertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)vertices.data());

    // staging to host
    _VertexBuffer = BufferPtr(new Buffer(
        _VulkanApp,
        vertexSize,
        _VertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ));
    VkDeviceSize bufferSize = sizeof(vertices[0]) * _VertexCount;
    _VulkanApp->copyBuffer(stagingBuffer.getBuffer(), _VertexBuffer->getBuffer(), bufferSize);

    // cleaning staging buffer
    stagingBuffer.cleanUp();
}

void Model::createIndexBuffer(const std::vector<uint32_t>& indices){
    _IndexCount = static_cast<uint32_t>(indices.size());
    if(_IndexCount == 0){
        _HasIndexBuffer = false;
        return;
    }
    _HasIndexBuffer = true;

    uint32_t indexSize = sizeof(indices[0]);

    // buffer to staging
    Buffer stagingBuffer{
        _VulkanApp,
        indexSize,
        _IndexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT  
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)indices.data());

    // staging to host
    _IndexBuffer = BufferPtr(new Buffer(
        _VulkanApp,
        indexSize,
        _IndexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ));
    VkDeviceSize bufferSize = sizeof(uint32_t) * _IndexCount;
    _VulkanApp->copyBuffer(stagingBuffer.getBuffer(), _IndexBuffer->getBuffer(), bufferSize);

    // cleaning staging buffer
    stagingBuffer.cleanUp();
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

                vertex._Col = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                    1.f
                };
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
	std::ifstream file(filePath.c_str());
    if(!file){
        ErrorHandler::handle(
            ErrorCode::IO_ERROR,
            "Failed to load off model: " + filePath + "!\n"
        );
    }
	
    std::string line = "";
    std::getline(file, line);
    if(line.find("OFF") == std::string::npos){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Trying to load a non .off file: " + filePath + "!\n"
        );
    }

    std::getline(file, line);

    std::istringstream iss(line);
    int numVertices = 0;
    int numFaces = 0;
    int numCells = 0;
    iss >> numVertices >> numFaces >> numCells;

    if(!_Vertices.empty()) _Vertices.clear();
    if(!_Indices.empty()) _Indices.clear();

    // vertices
    for(int i = 0; i < numVertices; i++){
        VertexData vertex{};
        std::getline(file, line);
        std::istringstream vertIss(line);
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
        vertIss >> x >> y >> z;
        vertex._Pos = glm::vec3(x, y, z);
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

    // normals
	for(int i=0; i<int(_Indices.size()); i+=3){
        // assuming triangle meshes
        int v0 = _Indices[i];
        int v1 = _Indices[i+1];
        int v2 = _Indices[i+2];

		glm::vec3 e0 = _Vertices[v1]._Pos - _Vertices[v0]._Pos;
		glm::vec3 e1 = _Vertices[v2]._Pos - _Vertices[v0]._Pos;
		glm::vec3 n = normalize (cross (e0, e1));
		for(int j=0; j<3; j++){
            _Vertices[_Indices[i+j]]._Norm += n;
        }
	}
	for(auto& vert : _Vertices){
        vert._Norm = glm::normalize(vert._Norm); 
    }

    file.close();
}

const std::map<std::string, Model::ModelExtension> Model::MODEL_EXTENSIONS_MAP = {
    {"off", OFF},
    {"obj", OBJ},
};

Model::Model(VulkanAppPtr vulkanApp, const std::string& filePath)
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

};