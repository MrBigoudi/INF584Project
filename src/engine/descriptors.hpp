#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "vulkanApp.hpp"

namespace BE{


class DescriptorSetLayout;
using DescriptorSetLayoutPtr = std::shared_ptr<DescriptorSetLayout>;

class DescriptorSetLayout {
    private:
        VulkanAppPtr _VulkanApp = nullptr;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;

    public:
        class Builder {
            private:
                VulkanAppPtr _VulkanApp;
                std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};

            public:
                Builder(VulkanAppPtr vulkanApp) : _VulkanApp{vulkanApp}{}

                Builder& addBinding(
                    uint32_t binding,
                    VkDescriptorType descriptorType,
                    VkShaderStageFlags stageFlags,
                    uint32_t count = 1
                );

                std::unique_ptr<DescriptorSetLayout> build() const;
        };

        DescriptorSetLayout(
            VulkanAppPtr vulkanApp, 
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings
        );

        VkDescriptorSetLayout getDescriptorSetLayout() const { 
            return descriptorSetLayout; 
        }

        void cleanUp(){
            vkDestroyDescriptorSetLayout(_VulkanApp->getDevice(), descriptorSetLayout, nullptr);
        }
};

class DescriptorPool;
using DescriptorPoolPtr = std::shared_ptr<DescriptorPool>;

class DescriptorPool {
    private:
        VulkanAppPtr _VulkanApp;
        VkDescriptorPool descriptorPool;

        friend class DescriptorWriter;

    public:
        class Builder {
            private:
                VulkanAppPtr _VulkanApp;
                std::vector<VkDescriptorPoolSize> poolSizes{};
                uint32_t maxSets = 1000;
                VkDescriptorPoolCreateFlags poolFlags = 0;

            public:
                Builder(VulkanAppPtr vulkanApp) : _VulkanApp{vulkanApp} {}

                Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
                Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
                Builder& setMaxSets(uint32_t count);
                DescriptorPoolPtr build() const;
        };

        DescriptorPool(
            VulkanAppPtr vulkanApp,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes
        );
    
        bool allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

        void cleanUp(){
            vkDestroyDescriptorPool(_VulkanApp->getDevice(), descriptorPool, nullptr);
        }
};



class DescriptorWriter {
    private:
        DescriptorSetLayout &setLayout;
        DescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;

    public:
        DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

        DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);
};

};