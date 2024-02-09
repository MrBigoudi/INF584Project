#include "gameObject.hpp"
#include "errorHandler.hpp"
#include <string>

GameObjectManagerPtr GameObjectManager::_GameObjectManager = nullptr;

GameObject GameObjectManager::createObject(){
    GameObjectManagerPtr instance = GameObjectManager::manager();
    if(instance->_LivingObjects >= MAX_NB_GAME_OBJECTS){
        ErrorHandler::handle(
            ErrorCode::OUT_OF_RANGE_ERROR, 
            "Too many objects, can't create a new one!\n"
        );
    }

    // take id from the queue
    GameObject id = instance->_AvailableIDs.front();
    instance->_AvailableIDs.pop();
    instance->_LivingObjects++;
    return id;
}

void GameObjectManager::destroyObject(GameObject object){
    GameObjectManagerPtr instance = GameObjectManager::manager();
    if(object >= MAX_NB_GAME_OBJECTS){
        ErrorHandler::handle(
            ErrorCode::OUT_OF_RANGE_ERROR, 
            "Can't destroy object " + std::to_string(object) + " max id is " + std::to_string(MAX_NB_GAME_OBJECTS) + "!\n",
            ErrorLevel::WARNING
        );
        return;
    }
    instance->_Signatures[object].reset();
    instance->_AvailableIDs.push(object);
    instance->_LivingObjects--;
}

void GameObjectManager::setSignature(GameObject object, GameObjectSignature signature){
    GameObjectManagerPtr instance = GameObjectManager::manager();
    if(object >= MAX_NB_GAME_OBJECTS){
        ErrorHandler::handle(
            ErrorCode::OUT_OF_RANGE_ERROR, 
            "Can't create signature for object " + std::to_string(object) + " max id is " + std::to_string(MAX_NB_GAME_OBJECTS) + "!\n",
            ErrorLevel::WARNING
        );
        return;
    }
    instance->_Signatures[object] = signature;
}

GameObjectSignature GameObjectManager::getSignature(GameObject object){
    GameObjectManagerPtr instance = GameObjectManager::manager();
    if(object >= MAX_NB_GAME_OBJECTS){
        ErrorHandler::handle(
            ErrorCode::OUT_OF_RANGE_ERROR, 
            "Can't get signature from object " + std::to_string(object) + " max id is " + std::to_string(MAX_NB_GAME_OBJECTS) + "!\n"
        );
    }
    return instance->_Signatures[object];
}