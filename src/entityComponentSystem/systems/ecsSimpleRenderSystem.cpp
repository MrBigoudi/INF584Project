#include "ecsSimpleRenderSystem.hpp"

#include "frameInfo.hpp"
#include "gameCoordinator.hpp"
#include "entity.hpp"
#include "gameObject.hpp"
#include "model.hpp"

#include "simpleRenderSubSystem.hpp"


void ECSSimpleRenderSystem::renderGameObjects(FrameInfo frameInfo, PipelinePtr pipeline, VkPipelineLayout pipelineLayout){
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
        ModelPtr model = GameCoordinator::getComponent<EntityModel>(object)._Model;

        SimplePushConstantData push{};
        push._Random = static_cast<float>(glfwGetTime());
        auto objectTransform = GameCoordinator::getComponent<EntityTransform>(object);
        push._Model = objectTransform.getModel();
        
        vkCmdPushConstants(
            frameInfo._CommandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(SimplePushConstantData), 
            &push
        );

        model->bind(frameInfo._CommandBuffer);
        model->draw(frameInfo._CommandBuffer);
    }
}

void ECSSimpleRenderSystem::cleanUpGameObjects(){
    for(auto object : _Objects){
        ModelPtr model = GameCoordinator::getComponent<EntityModel>(object)._Model;
        model->cleanUp();
        GameCoordinator::destroyObject(object);
    }
}
