#pragma once
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLM/glm.hpp>
#include <GLM/gtc/constants.hpp>
#include <memory>
#include <vector>


namespace SE {
struct VulkanContext;

class SeModel
{
public:

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    
    SeModel(std::shared_ptr<VulkanContext> inctx, std::vector<Vertex>& vertices);
    ~SeModel();
    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

    VkBuffer getBuffer() const { return storageBuffer; }
    uint32_t getVertexCount() const { return vertexCount; }

    

    private:
    void createVertexBuffer(const std::vector<Vertex>& vertices);
    void createStorageBuffer(const std::vector<Vertex>& vertices);
    std::shared_ptr<VulkanContext> ctx;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer storageBuffer;
    VkDeviceMemory storageBufferMemory;
    uint32_t vertexCount;
    
};
}

