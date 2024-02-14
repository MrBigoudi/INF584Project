#include "ecstest.hpp"

#include "componentRenderSubSystem.hpp"
#include "frameInfo.hpp"
#include "gameCoordinator.hpp"
#include "components.hpp"
#include "gameObject.hpp"
#include "model.hpp"

#include "globalFrameRenderSubSystem.hpp"


void ECStest::renderGameObjects(FrameInfo frameInfo, PipelinePtr pipeline, VkPipelineLayout pipelineLayout){
    pipeline->bind(frameInfo._CommandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo._CommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo._GlobalDescriptorSet,
        0,
        nullptr
    );

    for(auto const& object : _Objects){
        // temporary
        auto rss = GameCoordinator::getComponent<ComponentRenderSubSystem>(object);
        if(rss._RenderSubSystem->getPipeline() != pipeline) continue;

        ModelPtr model = GameCoordinator::getComponent<ComponentModel>(object)._Model;

        GlobalFramePushConstantData push{};
        auto objectTransform = GameCoordinator::getComponent<ComponentTransform>(object);
        push._Model = objectTransform.getModel();
        
        vkCmdPushConstants(
            frameInfo._CommandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(GlobalFramePushConstantData), 
            &push
        );

        model->bind(frameInfo._CommandBuffer);
        model->draw(frameInfo._CommandBuffer);
    }
}

void ECStest::cleanUpGameObjects(){
    for(auto object : _Objects){
        ModelPtr model = GameCoordinator::getComponent<ComponentModel>(object)._Model;
        model->cleanUp();
        GameCoordinator::destroyObject(object);
    }
}
