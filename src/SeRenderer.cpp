#include "SeRenderer.h"

#include <array>
#include <iostream>
#include <stdexcept>

#include "Config.h"
#include "SeDevice.h"
#include "SeObject.h"
#include "SePipeline.h"

namespace SE {

struct SimplePushConstantData
{
    glm::mat4 transform{1.f};
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

    ctx->Se_swapchain->clearOldSwapChain();
    ctx->Se_swapchain = new SeSwapChain(ctx, ctx->Se_swapchain);
    bool swapChainsFormatsIdentical = ctx->Se_swapchain->compareOldSwapFormats();
    if (!swapChainsFormatsIdentical) throw std::runtime_error("swapchain formats not identical!");

    std::cout << "Swapchain Recreated \n" << "New Swapchain format identical? " << (swapChainsFormatsIdentical ? "true":"false") << std::endl;    

    /*
    ctx->Se_swapchain->cleanupSwapChain();
    ctx->Se_swapchain->createSwapChain();
    ctx->Se_swapchain->createImageViews();
    ctx->Se_swapchain->createDepthResources();
    ctx->Se_swapchain->createFramebuffers();
    */
    
    //createPipeline();
}

void SeRenderer::createCommandBuffers()
{
    command_buffers.resize(SeSwapChain::MAX_FRAMES_IN_FLIGHT);

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

void SeRenderer::renderObjects(VkCommandBuffer commandBuffer)
{
    
    ctx->Se_pipeline->bind(commandBuffer);
    for (auto& obj: objects)
    {
        obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.0001f, glm::two_pi<float>());
        obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.0001f, glm::two_pi<float>());
        
        SimplePushConstantData push{};
        push.color = obj.color;
        push.transform = obj.transform.mat4();

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

VkCommandBuffer SeRenderer::beginFrame()
{
    assert(!isFrameInProgress() && "Cannot call beginFrame() while frame is already in progress!");
    // Grab a swap chain image
    auto result = ctx->Se_swapchain->acquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(currentImageIndex);
        ctx->Se_pipeline->recreateGraphicsPipeline();
        return nullptr;
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    bFrameInProgress = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin command buffer recording!");
    }

    return commandBuffer;
}

void SeRenderer::endFrame()
{
    assert(isFrameInProgress() && "No frames are in progress to end.");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) { throw std::runtime_error("failed to end command buffer recording!"); }
    bFrameInProgress = false;
    auto result = ctx->Se_swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || ctx->Se_window->framebufferResized)
    {
        ctx->Se_window->framebufferResized = false;
        recreateSwapChain(currentImageIndex);
        ctx->Se_pipeline->recreateGraphicsPipeline();
        
        return;
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit swap chain command buffers!");
    }
    currentFrameIndex = (currentFrameIndex + 1) % SeSwapChain::MAX_FRAMES_IN_FLIGHT; 
    
}

void SeRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameInProgress() && "Cannot call beginSwapChainRenderPass() while frame is already in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin renderpass on command buffer from a different frame");

    // RenderPass
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = ctx->Se_swapchain->render_pass;
    renderPassInfo.framebuffer = ctx->Se_swapchain->getFrameBuffer(currentImageIndex);
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = ctx->Se_swapchain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Dynamic Viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(ctx->Se_swapchain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(ctx->Se_swapchain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, ctx->Se_swapchain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
}

void SeRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(isFrameInProgress() && "Cannot call endSwapChainRenderPass() while frame is already in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end renderpass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
    
}

void SeRenderer::loadModel()
{
}

void SeRenderer::loadObjects()
{
    loadCubeModel({0.f, 0.f, 0.f});
    SeObject cube = SeObject::createObject();
    cube.model = ctx->Se_model;
    cube.transform.translation = { 0.f, 0.f, 0.5f };
    cube.transform.scale = { 0.5f, 0.5f, 0.5f };
    objects.push_back(std::move(cube));
}


// temporary helper function, creates a 1x1x1 cube centered at offset
void SeRenderer::loadCubeModel(glm::vec3 offset) {
    std::vector<SeModel::Vertex> vertices{

        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

    };
    

    
    for (auto& v : vertices) {
    v.position += offset;
    }
    ctx->Se_model = std::make_shared<SeModel>(ctx, vertices);
}


}
