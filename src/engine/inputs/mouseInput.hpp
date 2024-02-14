#pragma once

#include "window.hpp"
#include "camera.hpp"

#include <memory>

namespace BE{

class MouseInput;
using MouseInputPtr = std::shared_ptr<MouseInput>;

enum MouseMode{
    MOUSE_MODE_CAMERA,
    MOUSE_MODE_WINDOW,
};

class MouseInput{

    private:
        static MouseInputPtr _MouseInput;
        CameraPtr _Camera = nullptr;
        WindowPtr _Window = nullptr;

        bool _FirstMouse = true;
        double _LastMouseX = 0.f;
        double _LastMouseY = 0.f;

        MouseMode _MouseMode = MOUSE_MODE_CAMERA;
        bool _IsMouseModeKeyPressed = false;

    private:
        MouseInput(){};
        static MouseInputPtr get(){
            if(!_MouseInput){
                _MouseInput = MouseInputPtr(new MouseInput());
            }
            return _MouseInput;
        }

    public:
        static void setMouseCallback(CameraPtr camera, WindowPtr window){
            get()->_Camera = camera;
            get()->_Window = window;
            glfwSetCursorPosCallback(window->getWindow(), mouseCallback);
        }

        static void mouseCallback(GLFWwindow* window[[maybe_unused]], double xpos, double ypos) {
            auto mouse = get();
            if(mouse->_MouseMode != MOUSE_MODE_CAMERA) return;

            if (mouse->_FirstMouse) {
                mouse->_LastMouseX = xpos;
                mouse->_LastMouseY = ypos;
                mouse->_FirstMouse = false;
            }

            double xoffset = mouse->_LastMouseX - xpos;
            double yoffset = mouse->_LastMouseY - ypos;

            mouse->_LastMouseX = xpos;
            mouse->_LastMouseY = ypos;

            mouse->_Camera->ProcessMouseMovement(
                static_cast<float>(xoffset), 
                static_cast<float>(yoffset)
            );
        }

        static void updateMouseMode(){
            auto mouse = get();
            if(!mouse->_IsMouseModeKeyPressed){
                mouse->_IsMouseModeKeyPressed = true;
                if(mouse->_MouseMode == MOUSE_MODE_CAMERA){
                    glfwSetInputMode(mouse->_Window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    mouse->_MouseMode = MOUSE_MODE_WINDOW;
                } else {
                    glfwSetInputMode(mouse->_Window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    mouse->_MouseMode = MOUSE_MODE_CAMERA;
                }
            }
        }

        static void resetMouseModeKey(){
            auto mouse = get();
            mouse->_IsMouseModeKeyPressed = false;
        }
};

};