#pragma once

#include <memory>
#include <string>
#include <optional>
#include <vector>

class Pipeline;
using PipelinePtr = std::shared_ptr<Pipeline>;

class Pipeline{
    // shaders
    private:
        using GraphicsShaderCode = std::optional<std::vector<char>>;

        struct GraphicsShader{
            std::string _Path;
            GraphicsShaderCode _Code;

            bool exists(){
                return _Code.has_value();
            }

            static GraphicsShader init(const std::string& path){
                return {
                    ._Path = path,
                    ._Code = readShaderFile(path)
                };
            }
        };

        struct GraphicsShaders{
            GraphicsShader _VertexShader;
            GraphicsShader _FragmentShader;
            GraphicsShader _TesselationControlShader;
            GraphicsShader _TesselationEvaluationShader;
            GraphicsShader _GeometryShader;
        };

    private:
        GraphicsShaders _GraphicsShaders = {};

    public:
        Pipeline(const std::string& vert, const std::string& frag, 
            const std::string& control = "", 
            const std::string& evaluation = "",
            const std::string& geom = "");

    private:
        static std::vector<char> readShaderFile(const std::string& filepath);

        void createGraphicsShaders(
            const std::string& vert, const std::string& frag, 
            const std::string& control, 
            const std::string& evaluation,
            const std::string& geom
        );

        void createGraphicsPipeline(
            const std::string& vert, const std::string& frag, 
            const std::string& control, 
            const std::string& evaluation,
            const std::string& geom
        );

};