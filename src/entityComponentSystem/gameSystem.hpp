#pragma once

#include "gameObject.hpp"
#include "errorHandler.hpp"

#include <set>
#include <string>
#include <unordered_map>

class GameCoordinator;
class GameSystem;
using GameSystemPtr = std::shared_ptr<GameSystem>;
class GameSystemManager;
using GameSystemManagerPtr = std::shared_ptr<GameSystemManager>;

class GameSystem{
    friend GameSystemManager;
    
    public:
        std::set<GameObject> _Objects;
};

class GameSystemManager{

    friend GameCoordinator;

    private:
        static GameSystemManagerPtr _GameSystemManager;
        GameSystemManager(){};

        static GameSystemManagerPtr manager(){
            if(!_GameSystemManager){
                _GameSystemManager = GameSystemManagerPtr(new GameSystemManager());
            }
            return _GameSystemManager;
        }

    private:
        // map from system type string pointer to a signature
        std::unordered_map<const char*, GameObjectSignature> _Signatures{};
        // map from system type string pointer to a system pointer
        std::unordered_map<const char*, GameSystemPtr> _Systems{};

    private:
        template<typename T>
        static std::shared_ptr<T> registerSystem(){
            auto instance = manager();
            const char* typeName = typeid(T).name();

            if(instance->_Systems.find(typeName) != instance->_Systems.end()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "The system " + std::string(typeName) + " is already registered!\n"
                );
            }

            // create a pointer to the system and return it so it can be used externally
            auto system = std::make_shared<T>();
            instance->_Systems.insert({typeName, system});
            return system;
        }

        template<typename T>
        static void setSignature(GameObjectSignature signature){
            auto instance = manager();
            const char* typeName = typeid(T).name();

            if(instance->_Systems.find(typeName) == instance->_Systems.end()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "The system " + std::string(typeName) + " has not been registered!\n"
                );
            }

            // set the signature for this system
            instance->_Signatures.insert({typeName, signature});
        }

        static void gameObjectDestroyed(GameObject object){
            auto instance = manager();
            // erase a destroyed object from all system lists
            for (auto const& pair : instance->_Systems){
                auto const& system = pair.second;
                if(system->_Objects.contains(object))
                    system->_Objects.erase(object);
            }
        }

        static void objectSignatureChanged(GameObject object, GameObjectSignature objectSignature){
            auto instance = manager();
            // notify each system that an object's signature changed
            for (auto const& pair : instance->_Systems){
                auto const& type = pair.first;
                auto const& system = pair.second;
                auto const& systemSignature = instance->_Signatures[type];

                // object signature matches system signature - insert into set
                if ((objectSignature & systemSignature) == systemSignature){
                    system->_Objects.insert(object);
                }
                // object signature does not match system signature - erase from set
                else{
                    system->_Objects.erase(object);
                }
            }
        }

};