#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "renderer.hpp"
#include "window.hpp"
#include "vulkanApp.hpp"

namespace BE{

class IApplication{

    protected:
        WindowPtr _Window = nullptr;
        VulkanAppPtr _VulkanApp = nullptr;
        RendererPtr _Renderer = nullptr;

    private:
        virtual void init() = 0;
        virtual void cleanUp() = 0;
        virtual void mainLoop() = 0;

    public:
        IApplication(){};

        virtual void run(){
            init();
            mainLoop();
            cleanUp();
        }

        WindowPtr getWindow() const {
            return _Window;
        }
};

}