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
    void createPipeline();
    void createCommandBuffers();
    void recreateSwapChain(int imageIndex);
    void renderObjects(VkCommandBuffer commandBuffer, SeCamera &camera);
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
    
    std::vector<VkCommandBuffer> command_buffers;
    VkPipelineLayout pipeline_layout;
    std::shared_ptr<VulkanContext> ctx;
    std::vector<SeObject> objects;

private:
    void loadModel();
    void loadObjects();
    
    void updateFPS();
    
private:

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
