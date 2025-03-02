#include "SeRenderer.h"

#include <array>
#include <chrono>
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
struct RayTracePushConstantData {
    glm::mat4 projView;
    glm::vec2 resolution;
};

SeRenderer::SeRenderer(std::shared_ptr<VulkanContext> inctx)
{
    ctx = inctx;
    if (Config::get().ray_tracing()) {
        // Load objects first to ensure objects[0] is valid
        loadObjects();
        // Create descriptor set layout before pipeline layout
        createDescriptorSetLayout();
        createPipelineLayout();
        createComputePipeline();
         
        createDescriptorPool();
        createOutputImage(); 
        createDescriptorSet();
        createCommandBuffers();
    } else {
        loadObjects();
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }
    
}

SeRenderer::~SeRenderer()
{
    vkDestroyPipelineLayout(ctx->Se_device->device, pipeline_layout, nullptr);
    vkDestroyImageView(ctx->Se_device->device, outputImageView, nullptr);
    vkDestroyImage(ctx->Se_device->device, outputImage, nullptr);
    vkFreeMemory(ctx->Se_device->device, outputImageMemory, nullptr);
    vkDestroyDescriptorSetLayout(ctx->Se_device->device, descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(ctx->Se_device->device, descriptor_pool, nullptr);
    vkDestroyImage(ctx->Se_device->device, outputImage, nullptr);

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

    VkResult result = vkCreatePipelineLayout(ctx->Se_device->device, &pipelineLayoutInfo, VK_NULL_HANDLE, &pipeline_layout) ;
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    
}

void SeRenderer::createRayTracePipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(RayTracePushConstantData);

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptor_set_layout;
}

void SeRenderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding ssboBinding{};
    ssboBinding.binding = 0;
    ssboBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboBinding.descriptorCount = 1;
    ssboBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding imageBinding{};
    imageBinding.binding = 1;
    imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageBinding.descriptorCount = 1;
    imageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {ssboBinding, imageBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(ctx->Se_device->device, &layoutInfo, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void SeRenderer::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
    };
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(ctx->Se_device->device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void SeRenderer::createDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptor_set_layout;
    VkResult result = vkAllocateDescriptorSets(ctx->Se_device->device, &allocInfo, &descriptor_set);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    // Assume objects[0].model is your SeModel for simplicity
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = objects[0].model->getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(SeModel::Vertex) * objects[0].model->getVertexCount();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = outputImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    std::vector<VkWriteDescriptorSet> descriptorWrites(2);
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptor_set;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptor_set;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(ctx->Se_device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void SeRenderer::createOutputImage()
{
    VkExtent2D extent = ctx->Se_swapchain->getSwapChainExtent();
    uint32_t width = ctx->Se_swapchain->getSwapChainExtent().width;
    uint32_t height = ctx->Se_swapchain->getSwapChainExtent().height;

    // Create the image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Matches rgba8 in shader
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // Storage for compute, transfer for swapchain
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(ctx->Se_device->device, &imageInfo, nullptr, &outputImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create output image");
    }

    // Allocate memory for the image
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(ctx->Se_device->device, outputImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = ctx->Se_device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(ctx->Se_device->device, &allocInfo, nullptr, &outputImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate output image memory");
    }

    vkBindImageMemory(ctx->Se_device->device, outputImage, outputImageMemory, 0);

    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = outputImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(ctx->Se_device->device, &viewInfo, nullptr, &outputImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create output image view");
    }

    // Transition image layout to VK_IMAGE_LAYOUT_GENERAL
    VkCommandBuffer commandBuffer = ctx->Se_device->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = outputImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    ctx->Se_device->endSingleTimeCommands(commandBuffer);
}

void SeRenderer::createPipeline()
{
    ctx->Se_pipeline = new SePipeline(ctx);
    ctx->Se_pipeline->defaultPipelineConfigInfo();
    ctx->Se_pipeline->pipeline_config_info.renderPass = ctx->Se_swapchain->render_pass;
    ctx->Se_pipeline->pipeline_config_info.pipelineLayout = pipeline_layout;
    ctx->Se_pipeline->createGraphicsPipeline();

}

void SeRenderer::createComputePipeline()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(RayTracePushConstantData);

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptor_set_layout;

    if (vkCreatePipelineLayout(ctx->Se_device->device, &layoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    ctx->Se_pipeline = new SePipeline(ctx);
    ctx->Se_pipeline->defaultPipelineConfigInfo();
    ctx->Se_pipeline->pipeline_config_info.pipelineLayout = pipeline_layout;
    ctx->Se_pipeline->createComputePipeline();
    createOutputImage();
    createDescriptorPool();
    createDescriptorSet();
    
    
    
    
    
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

void SeRenderer::renderObjects(VkCommandBuffer commandBuffer, SeCamera &camera)
{
    ctx->Se_pipeline->bind(commandBuffer);

    auto projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();
    
    for (auto& obj: objects)
    {
        
        SimplePushConstantData push{};
        push.color = obj.color;
        push.transform = projectionView * obj.transform.mat4();

        vkCmdPushConstants(commandBuffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}

void SeRenderer::renderRays(VkCommandBuffer commandBuffer, SeCamera& camera)
{
    ctx->Se_pipeline->bind(commandBuffer);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    uint32_t width = ctx->Se_swapchain->getSwapChainExtent().width;
    uint32_t height = ctx->Se_swapchain->getSwapChainExtent().height;
    
    RayTracePushConstantData push{};
    push.projView = camera.getProjectionMatrix() * camera.getViewMatrix();
    push.resolution = {width, height};

    vkCmdPushConstants(commandBuffer, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(RayTracePushConstantData), &push);
    vkCmdDispatch(commandBuffer, (push.resolution.x + 15) / 16, (push.resolution.y + 15) / 16, 1);

     // Barrier: Wait for compute shader to finish writing to outputImage
    VkImageMemoryBarrier outputBarrier{};
    outputBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    outputBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    outputBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    outputBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    outputBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    outputBarrier.image = outputImage;
    outputBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    outputBarrier.subresourceRange.baseMipLevel = 0;
    outputBarrier.subresourceRange.levelCount = 1;
    outputBarrier.subresourceRange.baseArrayLayer = 0;
    outputBarrier.subresourceRange.layerCount = 1;
    outputBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    outputBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &outputBarrier
    );

    // Copy outputImage to swapchain image
    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.extent = {width, height, 1};

    vkCmdCopyImage(
        commandBuffer,
        outputImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        ctx->Se_swapchain->getSwapChainImage(currentImageIndex), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &copyRegion
    );

    // Transition outputImage back to GENERAL for the next frame
    VkImageMemoryBarrier outputBackBarrier{};
    outputBackBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    outputBackBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    outputBackBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    outputBackBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    outputBackBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    outputBackBarrier.image = outputImage;
    outputBackBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    outputBackBarrier.subresourceRange.baseMipLevel = 0;
    outputBackBarrier.subresourceRange.levelCount = 1;
    outputBackBarrier.subresourceRange.baseArrayLayer = 0;
    outputBackBarrier.subresourceRange.layerCount = 1;
    outputBackBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    outputBackBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    // Transition swapchain image to PRESENT_SRC_KHR
    VkImageMemoryBarrier swapchainBarrier{};
    swapchainBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    swapchainBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    swapchainBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    swapchainBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swapchainBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    swapchainBarrier.image = ctx->Se_swapchain->getSwapChainImage(currentImageIndex);
    swapchainBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    swapchainBarrier.subresourceRange.baseMipLevel = 0;
    swapchainBarrier.subresourceRange.levelCount = 1;
    swapchainBarrier.subresourceRange.baseArrayLayer = 0;
    swapchainBarrier.subresourceRange.layerCount = 1;
    swapchainBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    swapchainBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    // Combine both barriers in one call
    VkImageMemoryBarrier barriers[2] = {outputBackBarrier, swapchainBarrier};
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 2, barriers
    );
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
    updateFPS();
    // Grab a swap chain image
    auto result = ctx->Se_swapchain->acquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(currentImageIndex);
        if (Config::get().ray_tracing())ctx->Se_pipeline->recreateComputePipeline();
        else ctx->Se_pipeline->recreateGraphicsPipeline();
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


    // Transition swapchain image to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Assume initial state
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = ctx->Se_swapchain->getSwapChainImage(currentImageIndex); // Adjust based on your SeSwapChain API
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier
    );
    

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
        if (Config::get().ray_tracing())ctx->Se_pipeline->recreateComputePipeline();
        else  ctx->Se_pipeline->recreateGraphicsPipeline();
        
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
    cube.transform.translation = { 0.f, 0.f, 2.5f };
    cube.transform.scale = { 0.5f, 0.5f, 0.5f };
    objects.push_back(std::move(cube));

    SeObject cube2 = SeObject::createObject();
    cube2.model = ctx->Se_model;
    cube2.transform.translation = { 3.f, 0.f, 5.5f };
    cube2.transform.scale = { 0.5f, 0.5f, 0.5f };
    objects.push_back(std::move(cube2));
}

void SeRenderer::updateFPS()
{
    frameCount++;
    double currentTime = glfwGetTime();
    double elapsedTime = currentTime - lastFPSTime;

    // Update FPS every second
    if (elapsedTime >= 1.0) {
        avgFPS = frameCount / (float)elapsedTime;
        std::cout << "Average FPS: " << avgFPS << std::endl;
        
        // Reset counters
        frameCount = 0;
        lastFPSTime = currentTime;
    }
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
