#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "vulkancontext.h"

namespace SE {

class SePipeline
{
public:
    SePipeline(std::shared_ptr<VulkanContext> inctx);
    ~SePipeline();

    SePipeline(const SePipeline&) = delete;
    void operator=(const SePipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    
    void defaultPipelineConfigInfo(uint32_t width, uint32_t height);

    void createGraphicsPipeline();
    
private:
    static std::vector<char> readFile(std::string file);
    
    
    void createShaderModule(const std::vector<char>& code, VkShaderModule &shaderModule);

    std::shared_ptr<VulkanContext> pctx;
    
};
}
