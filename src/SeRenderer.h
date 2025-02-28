#pragma once
#include <memory>

#include "SePipeline.h"

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

    void drawFrame();

public:
    
    std::vector<VkCommandBuffer> command_buffers;
    VkPipelineLayout pipeline_layout;
    std::shared_ptr<VulkanContext> ctx;

private:
    void loadModel();

    
    
};

    
};
