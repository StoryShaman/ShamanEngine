#pragma once
// vulkan headers
#include <vulkan/vulkan.h>
#include "vulkancontext.h"
// std lib headers
#include <vector>

namespace SE {

class SeSwapChain {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SeSwapChain(std::shared_ptr<VulkanContext> inctx);
    ~SeSwapChain();

    SeSwapChain(const SeSwapChain &) = delete;
    SeSwapChain& operator=(const SeSwapChain &) = delete;

    void createSwapChain();
    void createFramebuffers();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createSyncObjects();

    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
    std::vector<VkFramebuffer> getFrameBuffers() { return swapChainFramebuffers; }
    VkRenderPass getRenderPass() { return renderPass; }
    VkImageView getImageView(int index) { return swapChainImageViews[index]; }
    std::vector<VkImageView> getImageViews() { return swapChainImageViews; }
    size_t imageCount() { return swapChainImages.size(); }
    VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }
    uint32_t width() { return swapChainExtent.width; }
    uint32_t height() { return swapChainExtent.height; }

    float extentAspectRatio() {
      return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

private:
    
    

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    
    VkRenderPass renderPass;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemorys;
    std::vector<VkImageView> depthImageViews;
    std::vector<VkImage> swapChainImages;
    
    

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    std::shared_ptr<VulkanContext> ctx;
};

}  // namespace lve
