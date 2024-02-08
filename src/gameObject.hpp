#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include <memory>
#include <queue>
#include <array>

class GameObjectManager;
using GameObjectManagerPtr = std::shared_ptr<GameObjectManager>;

static const uint32_t MAX_NB_GAME_OBJECTS = 8192;
using GameObject = uint32_t;
using GameObjectSignature = std::bitset<MAX_NB_GAME_OBJECTS>;


class GameObjectManager{

    private:
        GameObjectManager(){
            // init the queue
            for(GameObject object = 0; object < MAX_NB_GAME_OBJECTS; object++){
                _AvailableIDs.push(object);
            }
        }

    private:
        // singleton dp
        static GameObjectManagerPtr manager(){
            if(!_GameObjectManager){
                _GameObjectManager = GameObjectManagerPtr(new GameObjectManager());
            }
            return _GameObjectManager;
        }

    private:
        // queue of unused ids
        std::queue<GameObject> _AvailableIDs{};

        // array of signature where the index corresponds to the object ID
        std::array<GameObjectSignature, MAX_NB_GAME_OBJECTS> _Signatures{};

        // total living objects
        uint32_t _LivingObjects = 0;

        // singleton dp
        static GameObjectManagerPtr _GameObjectManager;

    public:
        static GameObject createObject();
        static void destroyObject(GameObject object);
        static void setSignature(GameObject object, GameObjectSignature signature);
        static GameObjectSignature getSignature(GameObject object);
};