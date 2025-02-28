#include "SeRenderer.h"

#include <array>
#include <iostream>
#include <stdexcept>

#include "SeDevice.h"
#include "SeObject.h"
#include "SePipeline.h"

namespace SE {

struct SimplePushConstantData
{
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

SeRenderer::SeRenderer(std::shared_ptr<VulkanContext> inctx)
{
    ctx = inctx;
    loadObjects();
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
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
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

    // RenderPass
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx->Se_swapchain->render_pass;
    renderPassInfo.framebuffer = ctx->Se_swapchain->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = ctx->Se_swapchain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(command_buffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Dynamic Viewport
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
    
    renderObjects(command_buffers[imageIndex]);

    vkCmdEndRenderPass(command_buffers[imageIndex]);
    if (vkEndCommandBuffer(command_buffers[imageIndex]) != VK_SUCCESS) { throw std::runtime_error("failed to end command buffer recording!"); }
    
}

void SeRenderer::renderObjects(VkCommandBuffer commandBuffer)
{
    
    
    ctx->Se_pipeline->bind(commandBuffer);
    for (auto& obj: objects)
    {
        obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());
        SimplePushConstantData push{};
        push.offset = obj.transform2d.translation;
        push.color = obj.color;
        push.transform = obj.transform2d.mat2();

        vkCmdPushConstants(commandBuffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
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
    // Grab a swap chain image
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

    
    // Record Command Buffer
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

void SeRenderer::loadObjects()
{
    std::vector<SeModel::Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, 
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    ctx->Se_model = std::make_shared<SeModel>(ctx, vertices);
    auto triangle = SeObject::createObject();
    triangle.model = ctx->Se_model;
    triangle.color = {0.1f, 0.8f, 0.1f};
    triangle.transform2d.translation.x = 0.2f;
    triangle.transform2d.scale = {2.f, .5f};
    triangle.transform2d.rotation = .25f * glm::two_pi<float>();

    objects.push_back(std::move(triangle));
    
}
}
