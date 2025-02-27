#pragma once
#include <memory>

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

    std::shared_ptr<VulkanContext> ctx;

private:
    void loadModel();

    
    
};

    
};
