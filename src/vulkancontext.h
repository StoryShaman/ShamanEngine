#pragma once
#include <memory>


#include "SeModel.h"
#include "SeRenderer.h"
#include "SeSwapChain.h"
#include "SeWindow.h"
#include "SePipeline.h"

namespace SE {

class SeWindow;
class SeDevice;
class SePipeline;
class SeSwapChain;

struct PipelineConfigInfo {
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkGraphicsPipelineCreateInfo graphicsPipelineInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

struct VulkanContext
{
    GLFWwindow* window = nullptr;
    SeWindow* Se_window = nullptr;
    
    SeDevice* Se_device = nullptr;
    SePipeline* Se_pipeline = nullptr;
    SeSwapChain* Se_swapchain = nullptr;
    SeRenderer* Se_renderer = nullptr;
    std::unique_ptr<SeModel> Se_model = nullptr;

    PipelineConfigInfo pipeline_config_info;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceFeatures physical_device_features;
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    VkPipeline pipeline_graphics;
    VkPipelineLayout pipeline_layout;
    VkSwapchainKHR swap_chain;
    VkRenderPass render_pass;
    VkQueue queue;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    std::vector<VkCommandBuffer> command_buffers;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSet descriptor_set;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;
    
};
    
}
