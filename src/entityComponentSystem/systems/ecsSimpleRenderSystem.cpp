#include "ecsSimpleRenderSystem.hpp"

#include "gameCoordinator.hpp"
#include "entity.hpp"
#include "gameObject.hpp"
#include "model.hpp"

#include "simpleRenderSubSystem.hpp"

void ECSSimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, PipelinePtr pipeline, VkPipelineLayout pipelineLayout){
    pipeline->bind(commandBuffer);

    auto viewProj = _Camera->getPerspective() * _Camera->getView();

    for(auto const& object : _Objects){
        ModelPtr model = GameCoordinator::getComponent<EntityModel>(object)._Model;
        if(!model) continue;

        SimplePushConstantData push{};
        push._Random = static_cast<float>(glfwGetTime());
        auto objectTransform = GameCoordinator::getComponent<EntityTransform>(object);
        push._Model = viewProj * objectTransform.getModel();
        
        vkCmdPushConstants(
            commandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(SimplePushConstantData), 
            &push
        );

        model->bind(commandBuffer);
        model->draw(commandBuffer);
    }
}

void ECSSimpleRenderSystem::cleanUpGameObjects(){
    for(auto const& object : _Objects){
        ModelPtr model = GameCoordinator::getComponent<EntityModel>(object)._Model;
        if(!model) continue;
        model->cleanUp();
        GameCoordinator::destroyObject(object);
    }
}
