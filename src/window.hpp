#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <cstdint>
#include <string>

#include "errorHandler.hpp"

class Window;
using WindowPtr = std::shared_ptr<Window>;

class Window{

    private:
        uint32_t _Width = 0;
        uint32_t _Height = 0;
        std::string _Title = "";
        GLFWwindow* _Window = nullptr;

    public:
        Window(uint32_t width, uint32_t height, const std::string& title)
            : _Width{width}, _Height{height}, _Title{title}{}

        void cleanUp(){
            glfwDestroyWindow(_Window);
            glfwTerminate();
        }

        bool shouldClose() const {
            return glfwWindowShouldClose(_Window);
        }

        void init(){
            if(!glfwInit()){
                ErrorHandler::glfwError("Failed to init GLFW!\n");
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            _Window = glfwCreateWindow(
                _Width, _Height, 
                _Title.c_str(), 
                nullptr, nullptr
            );

            if(_Window == nullptr){
                ErrorHandler::glfwError("Failed to init GLFW window!\n");
            }

            // Make the window's context current
            glfwMakeContextCurrent(_Window);
        }

        VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR& surface){
            return glfwCreateWindowSurface(instance, _Window, nullptr, &surface);
        }

};