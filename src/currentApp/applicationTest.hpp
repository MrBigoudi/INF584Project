#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <BigoudiEngine.hpp>

#include "renderSubSystems.hpp" // IWYU pragma: keep


class Application;
using ApplicationPtr = std::shared_ptr<Application>;

enum RenderingMode{
    RASTERIZING,
    RAY_TRACING,
};

class Application : public be::IApplication{

    public:
        static const uint32_t WINDOW_WIDTH = 1280;
        static const uint32_t WINDOW_HEIGHT = 720;

    private:
        be::DescriptorPoolPtr _GlobalPool = nullptr;
        be::DescriptorPoolPtr _GlobalPoolTmp = nullptr;

        std::vector<be::GameObject> _GameObjects = {};
        be::CameraPtr _Camera = nullptr;

        FrameRenderSubSystemPtr _RenderSubSystem = nullptr;
        BrdfRenderSubSystemPtr _BRDFRenderSubSystem = nullptr;
        RaytracingRenderSubSystemPtr _RaytracingRenderSubSystem = nullptr;

        be::ScenePtr _Scene = nullptr;
        bool _IsSwitchRenderingModeKeyPressed = false;
        RenderingMode _RenderingMode = RASTERIZING;
        be::RayTracerPtr _RayTracer = nullptr;
        be::FrameInfo _CurrentFrame = {};
        bool _Hasrun = false;

        

    private:
        // init systems
        void initWindow();
        void initVulkan();
        void initCamera();
        void initRenderer();
        void initDescriptors();
        void initSystems();
        void initRenderSubSystems();
        void initScene();
        void initRaytracer();

        // init objects
        void initGameObjectsFrame();
        void initGameObjectsRayTracingViewRectangle();
        void initGameObjectsRoom();
        void initGameObjectsEntities();
        void initGameObjects();
        void initLights();
        void initGUI();

        // clean ups
        void cleanUpGUI();
        void cleanUpGameObjects();
        void cleanUpRenderSubSystems();
        void cleanUpDescriptors();
        void cleanUpRenderer();
        void cleanUpVulkan();
        void cleanUpWindow();

    // private main functions
    private:
        void init() override;
        void cleanUp() override;
        void mainLoop() override;
        void renderRasterizer(float frameTime);
        void renderRayTracing(float frameTime);

    // helpers
    public:
        void updateSwitchRenderingModeKey();
        void resetSwitchRenderingModeKey();
        void switchRenderingMode();
        void runRaytracer();


    // public main functions
    public:
        Application(){};
        void run() override;

};