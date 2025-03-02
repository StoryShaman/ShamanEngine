#pragma once
#include <memory>

#include "SeCamera.h"
#include "SePipeline.h"

namespace SE {
class SeObject;
}

namespace SE {
struct VulkanContext;
}

namespace SE {
class SeRenderer
{
public:
    
    SeRenderer(std::shared_ptr<VulkanContext> inctx);
    ~SeRenderer();

    void createPipelineLayout();
    void createRayTracePipelineLayout();
    void createPipeline();
    void createComputePipeline();
    void createCommandBuffers();
    void recreateSwapChain(int imageIndex);
    void renderObjects(VkCommandBuffer commandBuffer, SeCamera &camera);
    void renderRays(VkCommandBuffer commandBuffer, SeCamera &camera);
    void freeCommandBuffers();
    void loadCubeModel(glm::vec3 offset);

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
    
    
    bool isFrameInProgress() const { return bFrameInProgress; }
    VkCommandBuffer getCurrentCommandBuffer() const
    {
        assert(isFrameInProgress() && "Cannot get frame index when frame not in progress");
        return command_buffers[currentFrameIndex];
    }
    
    int SeRenderer::getFrameIndex() const
    {
        assert(isFrameInProgress() && "Cannot get frame index when frame not in progress");
        return currentFrameIndex;
    }

    int getDeltaTime() { return deltaTime; }
    
public:
    
    std::shared_ptr<VulkanContext> ctx;
    std::vector<SeObject> objects;
    std::vector<VkCommandBuffer> command_buffers;
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;

    VkImage outputImage;
    VkDeviceMemory outputImageMemory;
    VkImageView outputImageView;
    

private:
    void loadModel();
    void loadObjects();
    
    void updateFPS();
    
private:

    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSet();
    void createOutputImage();

    

    
    // FPS tracking
    int frameCount = 0;         // Number of frames since last update
    double lastFPSTime = 0.0;   // Time of last FPS update
    float avgFPS = 0.0f;        // Calculated average FPS

    bool bFrameInProgress = false;
    uint32_t currentImageIndex;
    int currentFrameIndex = 0;
    int deltaTime = 0;
    
};



};
