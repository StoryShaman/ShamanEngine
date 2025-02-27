#pragma once
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <memory>
#include <vector>
#include <GLM/glm.hpp>

namespace SE {
struct VulkanContext;

class SeModel
{
public:

    struct Vertex
    {
        glm::vec2 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    
    SeModel(std::shared_ptr<VulkanContext> inctx, std::vector<Vertex>& vertices);
    ~SeModel();
    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

    

    private:
    void createVertexBuffer(const std::vector<Vertex>& vertices);
    std::shared_ptr<VulkanContext> ctx;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertexCount;
};
}

