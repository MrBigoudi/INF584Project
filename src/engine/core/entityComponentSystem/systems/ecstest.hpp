#pragma once

#include "frameInfo.hpp"
#include "gameSystem.hpp"
#include "pipeline.hpp"

#include <vulkan/vulkan.hpp>
#include <memory>

namespace BE{

class ECStest;
using ECStestPtr = std::shared_ptr<ECStest>;

class ECStest : public GameSystem{

    public:
        ECStest(){}

    public:
        void renderGameObjects(
            FrameInfo frameInfo, 
            PipelinePtr pipeline,
            VkPipelineLayout pipelineLayout
        );

        void cleanUpGameObjects();
};

};