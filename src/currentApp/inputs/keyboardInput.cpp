#include "keyboardInput.hpp"

#include "applicationTest.hpp"

KeyboardInputPtr KeyboardInput::_KeyboardInput = nullptr;

void KeyboardInput::switchRenderingMode(be::WindowPtr window, Application* app){
    if(glfwGetKey(window->getWindow(), KEY_SWITCH_RENDERING_MODE) == GLFW_PRESS){
        app->updateSwitchRenderingModeKey();
    }
    if(glfwGetKey(window->getWindow(), KEY_SWITCH_RENDERING_MODE) == GLFW_RELEASE){
        app->resetSwitchRenderingModeKey();
    }
}

void KeyboardInput::runRaytracer(be::WindowPtr window, Application* app){
    if(glfwGetKey(window->getWindow(), KEY_RUN_RAYTRACING) == GLFW_PRESS){
        app->runRaytracer();
    }
}