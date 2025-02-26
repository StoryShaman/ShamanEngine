#include "SeRenderer.h"

#include <stdexcept>

#include "SePipeline.h"

namespace SE {
SeRenderer::SeRenderer(VulkanContext* inctx)
{
    ctx = inctx;
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

SeRenderer::~SeRenderer()
{
    vkDestroyPipelineLayout(ctx->device, ctx->pipeline_layout, nullptr);
}

void SeRenderer::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(ctx->device, &pipelineLayoutInfo, VK_NULL_HANDLE, &ctx->pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    
}

void SeRenderer::createPipeline()
{
    ctx->Se_pipeline = new SePipeline(ctx);
    ctx->Se_pipeline->defaultPipelineConfigInfo(ctx->Se_swapchain->getSwapChainExtent().width, ctx->Se_swapchain->getSwapChainExtent().height);
    ctx->pipeline_config_info.renderPass = ctx->Se_swapchain->getRenderPass();
    ctx->pipeline_config_info.pipelineLayout = ctx->pipeline_layout;
    ctx->Se_pipeline->createGraphicsPipeline();
    //ctx->Se_pipeline = new SePipeline(ctx, pipelineConfig);
}

void SeRenderer::createCommandBuffers()
{
}

void SeRenderer::drawFrame()
{
}

}
