#include "SeRenderer.h"

#include <array>
#include <iostream>
#include <stdexcept>

#include "SeDevice.h"
#include "SePipeline.h"

namespace SE {
SeRenderer::SeRenderer(std::shared_ptr<VulkanContext> inctx)
{
    ctx = inctx;
    loadModel();
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

SeRenderer::~SeRenderer()
{
    vkDestroyPipelineLayout(ctx->Se_device->device, pipeline_layout, nullptr);
}

void SeRenderer::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(ctx->Se_device->device, &pipelineLayoutInfo, VK_NULL_HANDLE, &pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    
}

void SeRenderer::createPipeline()
{
    ctx->Se_pipeline = new SePipeline(ctx);
    ctx->Se_pipeline->defaultPipelineConfigInfo();
    ctx->Se_pipeline->pipeline_config_info.renderPass = ctx->Se_swapchain->render_pass;
    ctx->Se_pipeline->pipeline_config_info.pipelineLayout = pipeline_layout;
    ctx->Se_pipeline->createGraphicsPipeline();

}

void SeRenderer::recreateSwapChain(int imageIndex)
{
    VkExtent2D extent = {static_cast<uint32_t>(ctx->Se_window->width), static_cast<uint32_t>(ctx->Se_window->height)};
    while (extent.width == 0 || extent.height == 0)
    {
        extent = {static_cast<uint32_t>(ctx->Se_window->width), static_cast<uint32_t>(ctx->Se_window->height)};
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(ctx->Se_device->device);


    if (ctx->Se_swapchain->imageCount() != command_buffers.size())
    {
        freeCommandBuffers();
        createCommandBuffers();
    }
    
    ctx->Se_swapchain->cleanupSwapChain();
       
    ctx->Se_swapchain->createSwapChain();
    ctx->Se_swapchain->createImageViews();
    ctx->Se_swapchain->createDepthResources();
    ctx->Se_swapchain->createFramebuffers();
    
    //createPipeline();
}

void SeRenderer::createCommandBuffers()
{
    command_buffers.resize(ctx->Se_swapchain->imageCount());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = ctx->Se_device->command_pool;
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    if (vkAllocateCommandBuffers(ctx->Se_device->device, &allocInfo, command_buffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    
    
}

void SeRenderer::recordCommandBuffer(int imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffers[imageIndex], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin command buffer recording!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx->Se_swapchain->render_pass;
    renderPassInfo.framebuffer = ctx->Se_swapchain->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = ctx->Se_swapchain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(command_buffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(ctx->Se_swapchain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(ctx->Se_swapchain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, ctx->Se_swapchain->getSwapChainExtent()};
    
    vkCmdSetViewport(command_buffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(command_buffers[imageIndex], 0, 1, &scissor);

    ctx->Se_pipeline->bind(command_buffers[imageIndex]);
    ctx->Se_model->bind(command_buffers[imageIndex]);
    ctx->Se_model->draw(command_buffers[imageIndex]);
    // OLD: vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
    vkCmdEndRenderPass(command_buffers[imageIndex]);
    if (vkEndCommandBuffer(command_buffers[imageIndex]) != VK_SUCCESS)
    {
        
        throw std::runtime_error("failed to end command buffer recording!");
    }
    
}

void SeRenderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(
        ctx->Se_device->device,
        ctx->Se_device->command_pool,
        static_cast<uint32_t>(command_buffers.size()),
        command_buffers.data()
        );
    command_buffers.clear();
    
}

void SeRenderer::drawFrame()
{
    uint32_t imageIndex = 0;
    auto result = ctx->Se_swapchain->acquireNextImage(&imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(imageIndex);
        ctx->Se_pipeline->recreateGraphicsPipeline();
        return;
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    recordCommandBuffer(imageIndex);

    result = ctx->Se_swapchain->submitCommandBuffers(&command_buffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || ctx->Se_window->framebufferResized)
    {
        ctx->Se_window->framebufferResized = false;
        recreateSwapChain(imageIndex);
        ctx->Se_pipeline->recreateGraphicsPipeline();
        return;
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit swap chain command buffers!");
    }
    
    
}

void SeRenderer::loadModel()
{
    std::vector<SeModel::Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, 
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    ctx->Se_model = std::make_unique<SeModel>(ctx, vertices);
    std::cout << "Model Loaded";
}
}
