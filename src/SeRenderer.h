#pragma once
#include <memory>

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
    void renderObjects(VkCommandBuffer commandBuffer);
    void freeCommandBuffers();
    void loadCubeModel(glm::vec3 offset);

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
    
    
    void drawFrame();
    
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
    
public:
    
    std::vector<VkCommandBuffer> command_buffers;
    VkPipelineLayout pipeline_layout;
    std::shared_ptr<VulkanContext> ctx;
    std::vector<SeObject> objects;

private:
    void loadModel();
    void loadObjects();
    
    
private:

    bool bFrameInProgress = false;
    uint32_t currentImageIndex;
    int currentFrameIndex = 0;
    
    
};



};
