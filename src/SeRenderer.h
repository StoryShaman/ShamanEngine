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
    void recordCommandBuffer(int imageIndex);
    void renderObjects(VkCommandBuffer commandBuffer);
    void freeCommandBuffers();
    void drawFrame();

public:
    
    std::vector<VkCommandBuffer> command_buffers;
    VkPipelineLayout pipeline_layout;
    std::shared_ptr<VulkanContext> ctx;
    std::vector<SeObject> objects;

private:
    void loadModel();
    void loadObjects();

    
    
};
    
};
