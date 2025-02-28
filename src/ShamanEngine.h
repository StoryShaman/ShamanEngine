#pragma once
#include <memory>
#include <vector>
#include <vulkan_core.h>

namespace SE {
    struct VulkanContext;

    class ShamanEngine
{
public:
    ShamanEngine();
    ~ShamanEngine();
    void run();
    void init();
    
public:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::shared_ptr<VulkanContext> ctx;
    
private:
    void createInstance();
    bool checkValidationLayerSupport();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions();
    void setupDebugMessenger();
    std::vector<const char *> getRequiredExtensions();

private:
    
    
    
};
}
