#pragma once

#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "window.hpp"
#include "inputs/mouseInput.hpp"

namespace BE{

class KeyboardInput;
using KeyboardInputPtr = std::shared_ptr<KeyboardInput>;

class KeyboardInput{

    public:
        enum KeyMapping{
            LEFT = GLFW_KEY_A,
            RIGHT = GLFW_KEY_D,
            FORWARD = GLFW_KEY_W,
            BACKWARD = GLFW_KEY_S,
            UP = GLFW_KEY_UP,
            DOWN = GLFW_KEY_DOWN,
            MOUSE_MODE = GLFW_KEY_M,
        };

        static KeyboardInputPtr _KeyboardInput;

    private:
        KeyboardInput(){};
        static KeyboardInputPtr get(){
            if(!_KeyboardInput){
                _KeyboardInput = KeyboardInputPtr(new KeyboardInput());
            }
            return _KeyboardInput;
        }

    public:
        static void moveCamera(WindowPtr window, float dt, CameraPtr camera){   
            if(glfwGetKey(window->getWindow(), LEFT) == GLFW_PRESS){
                camera->processKeyboard(CameraMovement::LEFT, dt);
            }
            if(glfwGetKey(window->getWindow(), RIGHT) == GLFW_PRESS){
                camera->processKeyboard(CameraMovement::RIGHT, dt);
            }
            if(glfwGetKey(window->getWindow(), FORWARD) == GLFW_PRESS){
                camera->processKeyboard(CameraMovement::FORWARD, dt);
            }
            if(glfwGetKey(window->getWindow(), BACKWARD) == GLFW_PRESS){
                camera->processKeyboard(CameraMovement::BACKWARD, dt);
            }
            if(glfwGetKey(window->getWindow(), UP) == GLFW_PRESS){
                camera->processKeyboard(CameraMovement::UP, dt);
            }
            if(glfwGetKey(window->getWindow(), DOWN) == GLFW_PRESS){
                camera->processKeyboard(CameraMovement::DOWN, dt);
            }
        }

        static void updateMouseMode(WindowPtr window){
            if(glfwGetKey(window->getWindow(), MOUSE_MODE) == GLFW_PRESS){
                MouseInput::updateMouseMode();
            }
            if(glfwGetKey(window->getWindow(), MOUSE_MODE) == GLFW_RELEASE){
                MouseInput::resetMouseModeKey();
            }
        }
};

};