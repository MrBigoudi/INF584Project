#pragma once

#include <memory>

namespace BE{

class Application;
using ApplicationPtr = std::shared_ptr<Application>;

class VulkanApp;
using VulkanAppPtr = std::shared_ptr<VulkanApp>;

};