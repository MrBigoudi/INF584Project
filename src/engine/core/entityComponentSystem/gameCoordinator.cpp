#include "gameCoordinator.hpp"

#include "gameComponent.hpp"
#include "gameObject.hpp"
#include "gameSystem.hpp"
        
namespace BE{

// game object manager methods
GameObject GameCoordinator::createObject(){
    return GameObjectManager::createObject();
}

void GameCoordinator::destroyObject(GameObject object){
    GameObjectManager::destroyObject(object);
    GameComponentManager::gameObjectDestroyed(object);
    GameSystemManager::gameObjectDestroyed(object);
}
};