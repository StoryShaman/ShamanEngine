#pragma once

#include "SeWindow.h"

// std lib headers
#include <string>
#include <vector>


namespace SE {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};


class SeDevice {
public:
  


    SeDevice(std::shared_ptr<VulkanContext> inctx);
    ~SeDevice();
  
    // Not copyable or movable
    SeDevice(const SeDevice &) = delete;
    void operator=(const SeDevice &) = delete;
    SeDevice(SeDevice &&) = delete;
    SeDevice &operator=(SeDevice &&) = delete;
    
    

    SwapChainSupportDetails getSwapChainSupport();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies();
    VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates,
      VkImageTiling tiling,
      VkFormatFeatureFlags features);

  // Buffer Helper Functions
    void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);

public:
  
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkCommandPool command_pool;
    VkPhysicalDeviceProperties properties;
    VkInstance instance;

  

private:
  
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    
    
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  
    std::shared_ptr<VulkanContext> ctx;
};

} 