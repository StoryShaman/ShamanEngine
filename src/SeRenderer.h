#pragma once

namespace SE {
struct VulkanContext;
}

namespace SE {
class SeRenderer
{
public:
    SeRenderer(VulkanContext* inctx);
    ~SeRenderer();

    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();

    void drawFrame();

    VulkanContext* ctx;
    
};

    
};
