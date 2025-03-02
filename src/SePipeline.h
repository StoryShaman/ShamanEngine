#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "vulkancontext.h"

namespace SE {

class SePipeline
{
public:

    struct PipelineConfigInfo {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo;
        VkGraphicsPipelineCreateInfo graphicsPipelineInfo;
        VkComputePipelineCreateInfo computePipelineInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkDescriptorSetLayout descriptorSetLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };
    
    SePipeline(std::shared_ptr<VulkanContext> inctx);
    ~SePipeline();

    SePipeline(const SePipeline&) = delete;
    void operator=(const SePipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    
    void defaultPipelineConfigInfo();

    void createGraphicsPipeline();
    void recreateGraphicsPipeline();
    void createComputePipeline();
    void recreateComputePipeline();
    


public:

    VkPipeline pipeline;
    PipelineConfigInfo pipeline_config_info{};
    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;
    
private:

    

    
    static std::vector<char> readFile(std::string file);
    
    
    void createShaderModule(const std::vector<char>& code, VkShaderModule &shaderModule);

    std::shared_ptr<VulkanContext> ctx;
    
};
}
