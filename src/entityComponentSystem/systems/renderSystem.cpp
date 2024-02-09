#include "renderSystem.hpp"
#include "application.hpp"
#include "gameCoordinator.hpp"
#include "entity.hpp"
#include "gameObject.hpp"
#include "model.hpp"

void RenderSystem::renderGameObjects(const Application* app, VkCommandBuffer commandBuffer){
    app->_Pipeline->bind(commandBuffer);

    for(auto const& object : _Objects){
        SimplePushConstantData push{};
        push._Random = static_cast<float>(glfwGetTime());
        push._Model = GameCoordinator::getComponent<EntityTransform>(object).getModel();
        
        vkCmdPushConstants(
            commandBuffer, 
            app->_PipelineLayout, 
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

void RenderSystem::cleanUpGameObjects(){
    for(auto const& object : _Objects){
        ModelPtr model = GameCoordinator::getComponent<EntityModel>(object)._Model;
        model->cleanUp();
        GameCoordinator::destroyObject(object);
    }
}
