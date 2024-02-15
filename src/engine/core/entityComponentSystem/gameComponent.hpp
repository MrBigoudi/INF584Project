#pragma once

#include "errorHandler.hpp"
#include "gameObject.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <string>

namespace BE{

class GameCoordinator;
class GameComponentManager;
using GameComponentManagerPtr = std::shared_ptr<GameComponentManager>;

class IGameComponentArray{
    public:
        virtual ~IGameComponentArray() = default;
        virtual void gameObjectDestroyed(GameObject object) = 0;
};

template<typename T>
class GameComponentArray : public IGameComponentArray{
    friend GameComponentManager;

    private:
        // the packed array of components
        std::array<T, MAX_NB_GAME_OBJECTS> _GameComponentArray{};

        // map from an object ID to an array index.
        std::unordered_map<GameObject, size_t> _ObjectToIndexMap{};

        // Map from an array index to an object ID.
        std::unordered_map<size_t, GameObject> _IndexToObjectMap{};

        // Total size of valid entries in the array.
        size_t _Size = 0;

    private:
        void insert(GameObject object, const T& component){
            if(_ObjectToIndexMap.find(object) != _ObjectToIndexMap.end()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "Component added to same object more than once!\n",
                    ErrorLevel::WARNING
                );
                return;
            }

            size_t newIndex = _Size;
            _ObjectToIndexMap[object] = newIndex;
            _IndexToObjectMap[newIndex] = object;
            _GameComponentArray[newIndex] = component;
            _Size++;
        }

        void remove(GameObject object){
            if(_ObjectToIndexMap.find(object) == _ObjectToIndexMap.end()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "Removing unexisting component!\n",
                    ErrorLevel::WARNING
                );
                return;
            }

            // copy element at the end into deleted element's place
            size_t indexOfRemoved = _ObjectToIndexMap[object];
            size_t indexOfLastElement = _Size - 1;
            _GameComponentArray[indexOfRemoved] = _GameComponentArray[indexOfLastElement];

            // update maps
            GameObject objectOfLastComponent = _IndexToObjectMap[indexOfLastElement];
            _ObjectToIndexMap[objectOfLastComponent] = indexOfRemoved;
            _IndexToObjectMap[indexOfRemoved] = objectOfLastComponent;

            // remove old element
            _ObjectToIndexMap.erase(object);
            _IndexToObjectMap.erase(indexOfLastElement);
            _Size--;
        }

        T& get(GameObject object){
            if(_ObjectToIndexMap.find(object) == _ObjectToIndexMap.end()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "Retrieving unexisting element!\n"
                );
            }
            return _GameComponentArray[object];
        }

        void gameObjectDestroyed(GameObject object) override {
            if(_ObjectToIndexMap.find(object) != _ObjectToIndexMap.end())
                remove(object);
        }
};

class GameComponentManager{
    friend GameCoordinator;

    private:
        static GameComponentManagerPtr _GameComponentManager;

    private:
        // map from type string pointer to a component type
        std::unordered_map<const char*, GameComponentType> _GameComponentTypes{};

        // map from type string pointer to a component array
        std::unordered_map<const char*, std::shared_ptr<IGameComponentArray>> _GameComponentArrays{};

        // the component type to be assigned to the next registered component - starting at 0
        GameComponentType _NextGameComponentType{};

        // convenience function to get the statically casted pointer to the ComponentArray of type T
        template<typename T>
        std::shared_ptr<GameComponentArray<T>> getComponentArray(){
            const char* typeName = typeid(T).name();
            if(_GameComponentTypes.find(typeName) == _GameComponentTypes.end()){
                ErrorHandler::handle(
                    ErrorCode::NOT_INITIALIZED_ERROR,
                    "Component " + std::string(typeName) + " not registered before use!\n"
                );
            }

            return std::static_pointer_cast<GameComponentArray<T>>(_GameComponentArrays[typeName]);
        }

    private:
        GameComponentManager(){};

        static GameComponentManagerPtr manager(){
            if(!_GameComponentManager){
                _GameComponentManager = GameComponentManagerPtr(new GameComponentManager());
            }
            return _GameComponentManager;
        }

    private:
        template<typename T>
        static void registerComponent(){
            auto instance = manager();
            const char* typeName = typeid(T).name();
            if(instance->_GameComponentTypes.find(typeName) != instance->_GameComponentTypes.end()){
                ErrorHandler::handle(
                    ErrorCode::BAD_VALUE_ERROR,
                    "Component " + std::string(typeName) + " is already registered!\n",
                    ErrorLevel::WARNING
                );
                return;
            }

            // add this component type to the component type map
            instance->_GameComponentTypes.insert({typeName, instance->_NextGameComponentType});

            // create a ComponentArray pointer and add it to the component arrays map
            instance->_GameComponentArrays.insert({typeName, std::make_shared<GameComponentArray<T>>()});

            // increment the value so that the next component registered will be different
            instance->_NextGameComponentType++;
        }

        template<typename T>
        static GameComponentType getComponentType(){
            auto instance = manager();
            const char* typeName = typeid(T).name();
            if(instance->_GameComponentTypes.find(typeName) == instance->_GameComponentTypes.end()){
                ErrorHandler::handle(
                    ErrorCode::NOT_INITIALIZED_ERROR,
                    "Component " + std::string(typeName) + " not registered before use!\n"
                );
            }

            // return this component's type - used for creating signatures
            return instance->_GameComponentTypes[typeName];
        }

        template<typename T>
        static void addComponent(GameObject object, const T& component){
            auto instance = manager();
            // add a component to the array for an object
            instance->getComponentArray<T>()->insert(object, component);
        }

        template<typename T>
        static void removeComponent(GameObject object){
            auto instance = manager();
            // remove a component from the array for an object
            instance->getComponentArray<T>()->remove(object);
        }

        template<typename T>
        static T& getComponent(GameObject object){
            auto instance = manager();
            // get a reference to a component from the array for an object
            return instance->getComponentArray<T>()->get(object);
        }

        static void gameObjectDestroyed(GameObject object){
            auto instance = manager();
            // notify each component array that an object has been destroyed
            // if it has a component for that object, it will remove it
            for (auto const& pair : instance->_GameComponentArrays){
                auto const& component = pair.second;
                component->gameObjectDestroyed(object);
            }
        }
};

};