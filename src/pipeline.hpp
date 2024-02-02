#pragma once

#include <memory>
#include <string>
#include <optional>
#include <vector>

#include "vulkanApp.hpp"

class Pipeline;
using PipelinePtr = std::shared_ptr<Pipeline>;

struct PipelineConfigInfo{

};

class Pipeline{
    // shaders
    private:
        using GraphicsShaderCode = std::optional<std::vector<char>>;

        struct GraphicsShader{
            std::string _Path;
            GraphicsShaderCode _Code;

            bool exists(){return _Code.has_value();}

            void init(const std::string& path){
                _Path = path,
                _Code = readShaderFile(path);
            }
        };

        GraphicsShader _VertexShader = {};
        GraphicsShader _FragmentShader = {};
        GraphicsShader _TesselationControlShader = {};
        GraphicsShader _TesselationEvaluationShader = {};
        GraphicsShader _GeometryShader = {};

    public:
        Pipeline(){};
        
        void initVertexShader(const std::string& vert){_VertexShader.init(vert);} 
        void initFragmentShader(const std::string& frag){_FragmentShader.init(frag);} 
        void initControlShader(const std::string& cont){_TesselationControlShader.init(cont);} 
        void initEvaluationhader(const std::string& eval){_TesselationEvaluationShader.init(eval);} 
        void initGeometryShader(const std::string& geom){_GeometryShader.init(geom);} 
        
        void init();        

    private:
        static std::vector<char> readShaderFile(const std::string& filepath);

        void createGraphicsPipeline();

};