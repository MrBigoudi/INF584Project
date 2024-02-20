#pragma once

#include "brdfRenderSubSystem.hpp"

#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <BigoudiEngine.hpp>

#include "mouseInput.hpp"

class KeyboardInput;
using KeyboardInputPtr = std::shared_ptr<KeyboardInput>;

class KeyboardInput{

    public:
        enum KeyMapping{
            KEY_LEFT = GLFW_KEY_A,
            KEY_RIGHT = GLFW_KEY_D,
            KEY_FORWARD = GLFW_KEY_W,
            KEY_BACKWARD = GLFW_KEY_S,
            KEY_UP = GLFW_KEY_UP,
            KEY_DOWN = GLFW_KEY_DOWN,
            KEY_MOUSE_MODE = GLFW_KEY_M,

            KEY_SWITCH_PIPELINE = GLFW_KEY_P,
            KEY_WIREFRAME = GLFW_KEY_F1,
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
        static void moveCamera(be::WindowPtr window, be::CameraPtr camera){   
            if(glfwGetKey(window->getWindow(), KEY_LEFT) == GLFW_PRESS){
                camera->processKeyboard(be::CameraMovement::LEFT);
            }
            if(glfwGetKey(window->getWindow(), KEY_RIGHT) == GLFW_PRESS){
                camera->processKeyboard(be::CameraMovement::RIGHT);
            }
            if(glfwGetKey(window->getWindow(), KEY_FORWARD) == GLFW_PRESS){
                camera->processKeyboard(be::CameraMovement::FORWARD);
            }
            if(glfwGetKey(window->getWindow(), KEY_BACKWARD) == GLFW_PRESS){
                camera->processKeyboard(be::CameraMovement::BACKWARD);
            }
            if(glfwGetKey(window->getWindow(), KEY_UP) == GLFW_PRESS){
                camera->processKeyboard(be::CameraMovement::UP);
            }
            if(glfwGetKey(window->getWindow(), KEY_DOWN) == GLFW_PRESS){
                camera->processKeyboard(be::CameraMovement::DOWN);
            }
        }

        static void updateMouseMode(be::WindowPtr window){
            if(glfwGetKey(window->getWindow(), KEY_MOUSE_MODE) == GLFW_PRESS){
                MouseInput::updateMouseMode();
            }
            if(glfwGetKey(window->getWindow(), KEY_MOUSE_MODE) == GLFW_RELEASE){
                MouseInput::resetMouseModeKey();
            }
        }

        static void switchPipeline(be::WindowPtr window, BrdfRenderSubSystemPtr rss){
            if(glfwGetKey(window->getWindow(), KEY_WIREFRAME) == GLFW_PRESS){
                rss->updateWireframePipelineKey();
            }
            if(glfwGetKey(window->getWindow(), KEY_WIREFRAME) == GLFW_RELEASE){
                rss->resetWireframePipelineKey();
            }
            if(glfwGetKey(window->getWindow(), KEY_SWITCH_PIPELINE) == GLFW_PRESS){
                rss->updateSwitchPipelineKey();
            }
            if(glfwGetKey(window->getWindow(), KEY_SWITCH_PIPELINE) == GLFW_RELEASE){
                rss->resetSwitchPipelineKey();
            }
        }
};