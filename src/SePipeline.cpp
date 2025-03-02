#include "SePipeline.h"

#include <cassert>
#include <Config.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <vulkan/vk_enum_string_helper.h>

#include "SeDevice.h"
#include "SeModel.h"
#include "vulkancontext.h"

namespace SE {
SePipeline::SePipeline(std::shared_ptr<VulkanContext> inpctx)
{
    ctx = inpctx;
}

SePipeline::~SePipeline()
{
    if (vert_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(ctx->Se_device->device, vert_shader_module, nullptr);
        vert_shader_module = VK_NULL_HANDLE;
    }
    if (frag_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(ctx->Se_device->device, frag_shader_module, nullptr);
        frag_shader_module = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(ctx->Se_device->device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
}

void SePipeline::bind(VkCommandBuffer commandBuffer)
{
    if (Config::get().ray_tracing()) vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    else vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void SePipeline::defaultPipelineConfigInfo()
{
    //PipelineConfigInfo configInfo{};
    pipeline_config_info.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipeline_config_info.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeline_config_info.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
 

    // viewport
    pipeline_config_info.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipeline_config_info.viewportInfo.viewportCount = 1;
    pipeline_config_info.viewportInfo.pViewports = nullptr;
    pipeline_config_info.viewportInfo.scissorCount = 1;
    pipeline_config_info.viewportInfo.pScissors = nullptr;

    // rasterizationInfo
    pipeline_config_info.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_config_info.rasterizationInfo.depthClampEnable = VK_FALSE;
    pipeline_config_info.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    pipeline_config_info.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipeline_config_info.rasterizationInfo.lineWidth = 1.0f;
    pipeline_config_info.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipeline_config_info.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipeline_config_info.rasterizationInfo.depthBiasEnable = VK_FALSE;
    pipeline_config_info.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    pipeline_config_info.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    pipeline_config_info.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
 
    pipeline_config_info.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipeline_config_info.multisampleInfo.sampleShadingEnable = VK_FALSE;
    pipeline_config_info.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_config_info.multisampleInfo.minSampleShading = 1.0f;           // Optional
    pipeline_config_info.multisampleInfo.pSampleMask = nullptr;             // Optional
    pipeline_config_info.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    pipeline_config_info.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
    
    pipeline_config_info.colorBlendAttachment.blendEnable = VK_FALSE;
    pipeline_config_info.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    pipeline_config_info.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    pipeline_config_info.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    pipeline_config_info.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    pipeline_config_info.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    pipeline_config_info.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;      
    pipeline_config_info.colorBlendAttachment.colorWriteMask =  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;

    pipeline_config_info.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipeline_config_info.colorBlendInfo.logicOpEnable = VK_FALSE;
    pipeline_config_info.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    pipeline_config_info.colorBlendInfo.attachmentCount = 1;
    pipeline_config_info.colorBlendInfo.pAttachments = &pipeline_config_info.colorBlendAttachment;
    pipeline_config_info.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    pipeline_config_info.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    pipeline_config_info.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    pipeline_config_info.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

 
    pipeline_config_info.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipeline_config_info.depthStencilInfo.depthTestEnable = VK_TRUE;
    pipeline_config_info.depthStencilInfo.depthWriteEnable = VK_TRUE;
    pipeline_config_info.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    pipeline_config_info.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    pipeline_config_info.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
    pipeline_config_info.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    pipeline_config_info.depthStencilInfo.stencilTestEnable = VK_FALSE;
    pipeline_config_info.depthStencilInfo.front = {};  // Optional
    pipeline_config_info.depthStencilInfo.back = {};   // Optional

    pipeline_config_info.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    pipeline_config_info.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipeline_config_info.dynamicStateInfo.pDynamicStates = pipeline_config_info.dynamicStateEnables.data();
    pipeline_config_info.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(pipeline_config_info.dynamicStateEnables.size());
    pipeline_config_info.dynamicStateInfo.flags = 0;
    
    
}



std::vector<char> SePipeline::readFile(std::string filepath)
{
    std::ifstream file{filepath, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
    
}

void SePipeline::createGraphicsPipeline()
{
    
    if (pipeline_config_info.pipelineLayout == VK_NULL_HANDLE) std::cout << "Cannot create graphics pipeline: No pipelineLayout provided in configInfo \n"; 
    if (pipeline_config_info.renderPass == VK_NULL_HANDLE) std::cout << "Cannot create graphics pipeline: No renderPass provided in configInfo \n"; 

    // Create Shader Modules
    auto vertCode = readFile(Config::get().shader_path() + "simple_shader_vert.spv");
    createShaderModule(vertCode, vert_shader_module);
    auto fragCode = readFile(Config::get().shader_path() + "simple_shader_frag.spv");
    createShaderModule(fragCode, frag_shader_module);


    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vert_shader_module;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = frag_shader_module;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    auto bindingDescriptions = SeModel::Vertex::getBindingDescriptions();
    auto attributeDescriptions = SeModel::Vertex::getAttributeDescriptions();
    
    // VkPipelineVertexInputStateCreateInfo
    pipeline_config_info.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipeline_config_info.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    pipeline_config_info.vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    pipeline_config_info.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    pipeline_config_info.vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

/*
    pipeline_config_info.viewport.width = ctx->Se_swapchain->getSwapChainExtent().width;
    pipeline_config_info.viewport.height = ctx->Se_swapchain->getSwapChainExtent().height;
    pipeline_config_info.scissor.extent = ctx->Se_swapchain->getSwapChainExtent();

    // VkPipelineViewportStateCreateInfo 
    pipeline_config_info.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipeline_config_info.viewportInfo.viewportCount = 1;
    pipeline_config_info.viewportInfo.pViewports = &pipeline_config_info.viewport;
    pipeline_config_info.viewportInfo.scissorCount = 1;
    pipeline_config_info.viewportInfo.pScissors = &pipeline_config_info.scissor;
*/
    // VkGraphicsPipelineCreateInf 
    pipeline_config_info.graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_config_info.graphicsPipelineInfo.pNext = nullptr;
    pipeline_config_info.graphicsPipelineInfo.flags = 0;
    pipeline_config_info.graphicsPipelineInfo.stageCount = 2;
    pipeline_config_info.graphicsPipelineInfo.pStages = shaderStages;
    pipeline_config_info.graphicsPipelineInfo.pVertexInputState = &pipeline_config_info.vertexInputInfo;
    pipeline_config_info.graphicsPipelineInfo.pInputAssemblyState = &pipeline_config_info.inputAssemblyInfo;
    pipeline_config_info.graphicsPipelineInfo.pTessellationState = nullptr;
    pipeline_config_info.graphicsPipelineInfo.pViewportState = &pipeline_config_info.viewportInfo;
    pipeline_config_info.graphicsPipelineInfo.pRasterizationState = &pipeline_config_info.rasterizationInfo;
    pipeline_config_info.graphicsPipelineInfo.pMultisampleState = &pipeline_config_info.multisampleInfo;
    pipeline_config_info.graphicsPipelineInfo.pDepthStencilState = &pipeline_config_info.depthStencilInfo;
    pipeline_config_info.graphicsPipelineInfo.pColorBlendState = &pipeline_config_info.colorBlendInfo;
    pipeline_config_info.graphicsPipelineInfo.pDynamicState = &pipeline_config_info.dynamicStateInfo;
    pipeline_config_info.graphicsPipelineInfo.layout = pipeline_config_info.pipelineLayout;
    pipeline_config_info.graphicsPipelineInfo.renderPass = ctx->Se_swapchain->render_pass;
    pipeline_config_info.graphicsPipelineInfo.subpass = pipeline_config_info.subpass;
    pipeline_config_info.graphicsPipelineInfo.basePipelineIndex = -1;
    pipeline_config_info.graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkResult result = vkCreateGraphicsPipelines(ctx->Se_device->device, VK_NULL_HANDLE, 1, &pipeline_config_info.graphicsPipelineInfo, nullptr, &pipeline);
    string_VkResult(result);
    if (result != VK_SUCCESS)
    {
        std::cout << "Failed to create graphics pipeline: " << result << std::endl;
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    
}

void SePipeline::recreateGraphicsPipeline()
{
    if (frag_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(ctx->Se_device->device, frag_shader_module, nullptr);
        frag_shader_module = VK_NULL_HANDLE;
    }
    if (vert_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(ctx->Se_device->device, vert_shader_module, nullptr);
        vert_shader_module = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(ctx->Se_device->device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    createGraphicsPipeline();
    
}

void SePipeline::createComputePipeline()
{
    if (pipeline_config_info.pipelineLayout == VK_NULL_HANDLE) std::cout << "Cannot create graphics pipeline: No pipelineLayout provided in configInfo \n"; 
    
    // Create Shader Modules
    auto compCode = readFile(Config::get().shader_path() + "raytrace.spv");
    createShaderModule(compCode, vert_shader_module);


    VkPipelineShaderStageCreateInfo shaderStages;
    shaderStages.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStages.module = vert_shader_module;
    shaderStages.pName = "main";
    shaderStages.flags = 0;
    shaderStages.pNext = nullptr;
    shaderStages.pSpecializationInfo = nullptr;

    auto bindingDescriptions = SeModel::Vertex::getBindingDescriptions();
    auto attributeDescriptions = SeModel::Vertex::getAttributeDescriptions();
    
    // VkPipelineVertexInputStateCreateInfo
    pipeline_config_info.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipeline_config_info.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    pipeline_config_info.vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    pipeline_config_info.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    pipeline_config_info.vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    
    // VkGraphicsPipelineCreateInf 
    pipeline_config_info.computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_config_info.computePipelineInfo.stage = shaderStages;
    pipeline_config_info.computePipelineInfo.layout = pipeline_config_info.pipelineLayout;

    VkResult result = vkCreateComputePipelines(ctx->Se_device->device, VK_NULL_HANDLE, 1, &pipeline_config_info.computePipelineInfo, nullptr, &pipeline);
    string_VkResult(result);
    if (result != VK_SUCCESS)
    {
        std::cout << "Failed to create compute pipeline: " << result << std::endl;
        throw std::runtime_error("Failed to create compute pipeline");
    }
}

void SePipeline::recreateComputePipeline()
{
    if (vert_shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(ctx->Se_device->device, vert_shader_module, nullptr);
        vert_shader_module = VK_NULL_HANDLE;
    }
    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(ctx->Se_device->device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
    createComputePipeline();
}

void SePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule &shaderModule)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(ctx->Se_device->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        std::cout << "Failed to create shader module" << std::endl;
        throw std::runtime_error("Failed to create shader module");
    }
}
}

