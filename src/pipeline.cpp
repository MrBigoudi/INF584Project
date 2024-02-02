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

Pipeline::Pipeline(const std::string& vert, const std::string& frag, 
    const std::string& control, 
    const std::string& evaluation,
    const std::string& geom
    ){

    createGraphicsPipeline(vert, frag, control, evaluation, geom);

}


void Pipeline::createGraphicsShaders(
    const std::string& vert, const std::string& frag, 
    const std::string& control, 
    const std::string& evaluation,
    const std::string& geom
    ){
    
    _GraphicsShaders = {};
    _GraphicsShaders._VertexShader = GraphicsShader::init(vert);
    _GraphicsShaders._FragmentShader = GraphicsShader::init(frag);
    if(control != "")
        _GraphicsShaders._TesselationControlShader = GraphicsShader::init(control);
    if(evaluation != "")
        _GraphicsShaders._TesselationEvaluationShader = GraphicsShader::init(evaluation);
    if(geom != "")
        _GraphicsShaders._GeometryShader = GraphicsShader::init(geom);
}

void Pipeline::createGraphicsPipeline(
    const std::string& vert, const std::string& frag, 
    const std::string& control, 
    const std::string& evaluation,
    const std::string& geom
    ){
    createGraphicsShaders(vert, frag, control, evaluation, geom);

}
