#pragma once

#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_ONE
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

#include "types.hpp"
#include "window.hpp"
#include "swapChain.hpp"

#include "errorHandler.hpp"

class Renderer;
using RendererPtr = std::shared_ptr<Renderer>;

class Renderer{

    private:
        WindowPtr _Window = nullptr;
        VulkanAppPtr _VulkanApp = nullptr;
        SwapChainPtr _SwapChain = nullptr;
        std::vector<VkCommandBuffer> _CommandBuffers;
        uint32_t _CurrentImageIndex = 0;
        uint32_t _CurrentFrameIndex = 0;
        bool _IsFrameStarted = false;

    private:
        void recreateSwapChain();
        void initCommandBuffers();
        void initSwapChain();
        void cleanUpCommandBuffers();

    public:
        Renderer(WindowPtr window, VulkanAppPtr vulkanApp);
        void cleanUp();

        VkCommandBuffer beginFrame();
        void endFrame();

        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        bool isFrameInProgress() const {
            return _IsFrameStarted;
        }

        VkCommandBuffer getCurrentCommandBuffer() const {
            if(!_IsFrameStarted){
                ErrorHandler::handle(
                    ErrorCode::NOT_INITIALIZED_ERROR, 
                    "Can't get command buffer when frame not in progress!\n"
                );
            }
            return _CommandBuffers[_CurrentFrameIndex];
        }

        VkRenderPass getSwapChainRenderPass() const {
            return _SwapChain->getRenderPass();
        }

        float getSwapChainAspectRatio() const {
            return _SwapChain->getExtentAspectRatio();
        }

        uint32_t getFrameIndex() const {
            if(!_IsFrameStarted){
                ErrorHandler::handle(
                    ErrorCode::NOT_INITIALIZED_ERROR, 
                    "Can't get current frame index when frame not in progress!\n"
                );
            }
            return _CurrentFrameIndex;
        }

        
};