#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "errorHandler.hpp"
#include "types.hpp"

namespace BE{

class Pipeline;
class GraphicsShader;
using PipelinePtr = std::shared_ptr<Pipeline>;
using GraphicsShaderPtr = std::shared_ptr<GraphicsShader>;

struct PipelineConfigInfo{
    VkPipelineViewportStateCreateInfo _ViewportInfo{};
    VkPipelineInputAssemblyStateCreateInfo _InputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo _RasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo _MultisampleInfo{};
    VkPipelineDepthStencilStateCreateInfo _DepthStencilInfo{};
    VkPipelineColorBlendAttachmentState _ColorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo _ColorBlendingInfo{};

    VkPipelineDynamicStateCreateInfo _DynamicStateInfo{};
    std::vector<VkDynamicState> _DynamicStateEnables;

    VkPipelineLayout _PipelineLayout{};
    VkRenderPass _RenderPass{};
    uint32_t _Subpass = 0;
};

enum GraphicsShaderStage{
    VERTEX_STAGE,
    FRAGMENT_STAGE,
    GEOMETRY_STAGE,
    TESSELLATION_CONTROL_STAGE,
    TESSELLATION_EVALUATION_STAGE,
};

class GraphicsShader{
    private:        
        using GraphicsShaderCode = std::optional<std::vector<char>>;
        std::string _Path;
        GraphicsShaderCode _Code;
        VkShaderModule _Module;
        GraphicsShaderStage _Stage;

    public:
        GraphicsShader(GraphicsShaderStage stage, const std::string& path = ""){
            _Stage = stage;
            if(path != "")
                init(path);
        }

        VkShaderModule getModule() const {
            return _Module;
        }

        void createModule(const VulkanAppPtr vulkanApp);
        
        bool exists(){return _Code.has_value();}
        VkPipelineShaderStageCreateInfo getShaderStageCreateInfo(){
            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.module = _Module;
            shaderStageInfo.pName = "main";
            shaderStageInfo.flags = 0;
            shaderStageInfo.pSpecializationInfo = nullptr;
            shaderStageInfo.pNext = nullptr;
            switch(_Stage){
                case VERTEX_STAGE:
                    shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    break;
                case FRAGMENT_STAGE:
                    shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    break;
                case GEOMETRY_STAGE:
                    shaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                    break;
                case TESSELLATION_CONTROL_STAGE:
                    shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    break;
                case TESSELLATION_EVALUATION_STAGE:
                    shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    break;
                default:
                    ErrorHandler::handle(ErrorCode::UNKNOWN_VALUE_ERROR, "Unkown shader stage!\n");
                    break;
            }
            return shaderStageInfo;
        }

    private:
        static std::vector<char> readShaderFile(const std::string& filepath);
        void init(const std::string& path){
            _Path = path,
            _Code = readShaderFile(path);
        }
};

class Pipeline{

    public:
        static PipelineConfigInfo defaultPipelineConfigInfo();

    private:
        GraphicsShaderPtr _VertexShader = GraphicsShaderPtr(new GraphicsShader(VERTEX_STAGE));
        GraphicsShaderPtr _FragmentShader = GraphicsShaderPtr(new GraphicsShader(FRAGMENT_STAGE));
        GraphicsShaderPtr _TesselationControlShader = GraphicsShaderPtr(new GraphicsShader(TESSELLATION_CONTROL_STAGE));
        GraphicsShaderPtr _TesselationEvaluationShader = GraphicsShaderPtr(new GraphicsShader(TESSELLATION_EVALUATION_STAGE));
        GraphicsShaderPtr _GeometryShader = GraphicsShaderPtr(new GraphicsShader(GEOMETRY_STAGE));

        VulkanAppPtr _VulkanApp = nullptr;
        VkPipeline _GraphicsPipeline;

    public:
        Pipeline(VulkanAppPtr vulkanApp) : _VulkanApp(vulkanApp){
        };
        
        void initVertexShader(const std::string& vert){
            _VertexShader = GraphicsShaderPtr(
                                new GraphicsShader(VERTEX_STAGE, vert)
                            );
            _VertexShader->createModule(_VulkanApp);
        } 

        void initFragmentShader(const std::string& frag){
            _FragmentShader = GraphicsShaderPtr(
                                new GraphicsShader(FRAGMENT_STAGE, frag)
                            );
            _FragmentShader->createModule(_VulkanApp);
        } 

        void initTessellationShaders(const std::string& cont, const std::string& eval){
            _TesselationControlShader = GraphicsShaderPtr(
                                            new GraphicsShader(TESSELLATION_CONTROL_STAGE, cont)
                                        );
            _TesselationControlShader->createModule(_VulkanApp);
            _TesselationEvaluationShader = GraphicsShaderPtr(
                                                new GraphicsShader(TESSELLATION_EVALUATION_STAGE, eval)
                                            );
            _TesselationEvaluationShader->createModule(_VulkanApp);
        }

        void initGeometryShader(const std::string& geom){
            _GeometryShader = GraphicsShaderPtr(
                                new GraphicsShader(GEOMETRY_STAGE, geom)
                            );
            _GeometryShader->createModule(_VulkanApp);
        } 
        
        void init(PipelineConfigInfo configInfo);
        void cleanUp();   

        void bind(VkCommandBuffer commandBuffer);     

    private:
        void createGraphicsPipeline(PipelineConfigInfo configInfo);
        std::vector<VkPipelineShaderStageCreateInfo> createShaderStages() const {
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
            shaderStages.push_back(_VertexShader->getShaderStageCreateInfo());
            shaderStages.push_back(_FragmentShader->getShaderStageCreateInfo());
            if(_TesselationControlShader->exists()){
                shaderStages.push_back(_TesselationControlShader->getShaderStageCreateInfo());
                shaderStages.push_back(_TesselationEvaluationShader->getShaderStageCreateInfo());
            }
            if(_GeometryShader->exists()){
                shaderStages.push_back(_GeometryShader->getShaderStageCreateInfo());
            }
            return shaderStages;
        };

        uint32_t getNbShaderStages() const {
            return _VertexShader->exists()
                + _FragmentShader->exists()
                + _GeometryShader->exists()
                + _TesselationControlShader->exists()
                + _TesselationEvaluationShader->exists();
        }
};

};