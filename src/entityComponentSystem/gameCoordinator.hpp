#pragma once

#include "gameComponent.hpp"
#include "gameObject.hpp"
#include "gameSystem.hpp"

#include <memory>

class GameCoordinator{
    private:
        GameCoordinator(){};

    public:        
        // game object manager methods
        static GameObject createObject();
        static void destroyObject(GameObject object);
        
        // game component manager methods
        template<typename T>
        static void registerComponent(){
            GameComponentManager::registerComponent<T>();
        }

        template<typename T>
        static void addComponent(GameObject object, T component){
            GameComponentManager::addComponent(object, component);
            auto signature = GameObjectManager::getSignature(object);
            signature.set(GameComponentManager::getComponentType<T>(), true);
            GameObjectManager::setSignature(object, signature);
            GameSystemManager::objectSignatureChanged(object, signature);
        }

        template<typename T>
        static void removeComponent(GameObject object){
            GameComponentManager::removeComponent<T>(object);
            auto signature = GameObjectManager::getSignature(object);
            signature.set(GameComponentManager::getComponentType<T>(), false);
            GameObjectManager::setSignature(object, signature);
            GameSystemManager::objectSignatureChanged(object, signature);
        }

        template<typename T>
        static T& getComponent(GameObject object){
            return GameComponentManager::getComponent<T>(object);
        }

        template<typename T>
        static GameComponentType getComponentType(){
            return GameComponentManager::getComponentType<T>();
        }

        // game system manager methods
        template<typename T>
        static std::shared_ptr<T> registerSystem(){
            return GameSystemManager::registerSystem<T>();
        }

        template<typename T>
        static void setSystemSignature(GameObjectSignature signature){
            GameSystemManager::setSignature<T>(signature);
        }

};