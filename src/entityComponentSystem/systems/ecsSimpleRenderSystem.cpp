#include "ecsSimpleRenderSystem.hpp"

#include "gameCoordinator.hpp"
#include "entity.hpp"
#include "gameObject.hpp"
#include "model.hpp"

#include "simpleRenderSubSystem.hpp"

void ECSSimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, PipelinePtr pipeline, VkPipelineLayout pipelineLayout){
    pipeline->bind(commandBuffer);

    for(auto const& object : _Objects){
        SimplePushConstantData push{};
        push._Random = static_cast<float>(glfwGetTime());
        push._Model = GameCoordinator::getComponent<EntityTransform>(object).getModel();
        
        vkCmdPushConstants(
            commandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(SimplePushConstantData), 
            &push
        );

        ModelPtr model = GameCoordinator::getComponent<EntityModel>(object)._Model;
        model->bind(commandBuffer);
        model->draw(commandBuffer);
    }
}

void ECSSimpleRenderSystem::cleanUpGameObjects(){
    for(auto const& object : _Objects){
        ModelPtr model = GameCoordinator::getComponent<EntityModel>(object)._Model;
        model->cleanUp();
        GameCoordinator::destroyObject(object);
    }
}
