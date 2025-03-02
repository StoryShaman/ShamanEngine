#include "SeModel.h"

#include "SeDevice.h"
#include "vulkancontext.h"

namespace SE {
std::vector<VkVertexInputBindingDescription> SeModel::Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> SeModel::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}

SeModel::SeModel(std::shared_ptr<VulkanContext> inctx, std::vector<Vertex>& vertices)
{
    ctx = inctx;
    createVertexBuffer(vertices);
    createStorageBuffer(vertices);
}

SeModel::~SeModel()
{
    vkDestroyBuffer(ctx->Se_device->device, vertexBuffer, nullptr);
    vkFreeMemory(ctx->Se_device->device, vertexBufferMemory, nullptr);
}

void SeModel::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void SeModel::draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

void SeModel::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >=  3 && "Vertex count must be greater than 3");
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertexCount;
    ctx->Se_device->createBuffer(
        vertexBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer,
            vertexBufferMemory
        );
    void *data;
    vkMapMemory(ctx->Se_device->device, vertexBufferMemory, 0, vertexBufferSize, 0, &data);
    memcpy(data, vertices.data(), vertexBufferSize);
    vkUnmapMemory(ctx->Se_device->device, vertexBufferMemory);
    
}
void SeModel::createStorageBuffer(const std::vector<Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be greater than 3");
    VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;

    ctx->Se_device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, // Change to SSBO
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        storageBuffer,
        storageBufferMemory
    );

    void* data;
    vkMapMemory(ctx->Se_device->device, storageBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(ctx->Se_device->device, storageBufferMemory);
}
}
