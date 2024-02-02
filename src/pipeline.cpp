#include "pipeline.hpp"
#include "errorHandler.hpp"

#include <cstddef>
#include <fstream>

std::vector<char> Pipeline::readShaderFile(const std::string& filepath){
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if(!file.is_open()){
        ErrorHandler::handle(
            ErrorCode::IO_ERROR, 
            "Failed to open the file " + filepath + "!\n"
        );
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void Pipeline::createGraphicsPipeline(){
    if(!_VertexShader.exists() || !_FragmentShader.exists()){
        ErrorHandler::handle(
            ErrorCode::NOT_INITIALIZED_ERROR,
            "Can't create a graphics pipeline without a vertex and a fragment shader!\n"
        );
    }
}

void Pipeline::init(){
    createGraphicsPipeline();
}