#include "descriptors.hpp"
#include "errorHandler.hpp"
#include "vulkanApp.hpp"

#include <cassert>
#include <stdexcept>

namespace BE{


// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) 
    {
    
    if(_Bindings.count(binding) != 0){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Binding is already in use!\n"
        );
    }
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    _Bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
    return std::make_unique<DescriptorSetLayout>(_VulkanApp, _Bindings);
}

// *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(
    VulkanAppPtr vulkanApp, 
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings
    ) : _VulkanApp{vulkanApp}, _Bindings{bindings} {

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    VkResult result = vkCreateDescriptorSetLayout(
            _VulkanApp->getDevice(),
            &descriptorSetLayoutInfo,
            nullptr,
            &_DescriptorSetLayout
    );
    
    ErrorHandler::vulkanError(result, "Failed to create descriptor set layout!\n");
}




// *************** Descriptor Pool Builder *********************

DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
    _PoolSizes.push_back({descriptorType, count});
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
    _PoolFlags = flags;
    return *this;
}
DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
    _MaxSets = count;
    return *this;
}

DescriptorPoolPtr DescriptorPool::Builder::build() const {
    return DescriptorPoolPtr(new DescriptorPool(_VulkanApp, _MaxSets, _PoolFlags, _PoolSizes));
}


// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(
    VulkanAppPtr vulkanApp,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : _VulkanApp{vulkanApp} {

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    VkResult result = vkCreateDescriptorPool(_VulkanApp->getDevice(), &descriptorPoolInfo, nullptr, &_DescriptorPool);
    ErrorHandler::vulkanError(result, "Failed to create descriptor pool!\n");
}


bool DescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _DescriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope

    if (vkAllocateDescriptorSets(_VulkanApp->getDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
    vkFreeDescriptorSets(
        _VulkanApp->getDevice(),
        _DescriptorPool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data()
    );
}

void DescriptorPool::resetPool() {
    vkResetDescriptorPool(_VulkanApp->getDevice(), _DescriptorPool, 0);
}




// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool)
    : _SetLayout{setLayout}, _Pool{pool} {}

DescriptorWriter& DescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
    
    if(_SetLayout._Bindings.count(binding) != 1){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Layout does not contain specified binding!\n"
        );
    }

    auto& bindingDescription = _SetLayout._Bindings[binding];
    if(bindingDescription.descriptorCount != 1){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Binding single descriptor info, but binding expects multiple!\n"
        );
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    _Writes.push_back(write);
    return *this;
}

DescriptorWriter& DescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo *imageInfo) {

    if(_SetLayout._Bindings.count(binding) != 1){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Layout does not contain specified binding!\n"
        );
    }

    auto& bindingDescription = _SetLayout._Bindings[binding];
    if(bindingDescription.descriptorCount != 1){
        ErrorHandler::handle(
            ErrorCode::BAD_VALUE_ERROR,
            "Binding single descriptor info, but binding expects multiple!\n"
        );
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    _Writes.push_back(write);
    return *this;
}

bool DescriptorWriter::build(VkDescriptorSet &set) {
    bool success = _Pool.allocateDescriptor(_SetLayout.getDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet &set) {
    for (auto &write : _Writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(
        _Pool._VulkanApp->getDevice(), 
    _Writes.size(), 
    _Writes.data(), 
    0, 
    nullptr
    );
}

};