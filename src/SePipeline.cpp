#include "SePipeline.h"

#include <cassert>
#include <Config.h>
#include <fstream>
#include <ios>
#include <iostream>

#include "SeModel.h"
#include "vulkancontext.h"

namespace SE {
SePipeline::SePipeline(std::shared_ptr<VulkanContext> inpctx)
{
    pctx = inpctx;
}

SePipeline::~SePipeline()
{
    vkDestroyShaderModule(pctx->device, pctx->vert_shader_module, nullptr);
    vkDestroyShaderModule(pctx->device, pctx->frag_shader_module, nullptr);
    vkDestroyPipeline(pctx->device, pctx->pipeline_graphics, nullptr);
}

void SePipeline::bind(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pctx->pipeline_graphics);
}

void SePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
{
    //PipelineConfigInfo configInfo{};
    pctx->pipeline_config_info.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pctx->pipeline_config_info.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pctx->pipeline_config_info.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // viewport
    pctx->pipeline_config_info.viewport.x = 0.0f;
    pctx->pipeline_config_info.viewport.y = 0.0f;
    pctx->pipeline_config_info.viewport.width = static_cast<float>(width);
    pctx->pipeline_config_info.viewport.height = static_cast<float>(height);
    pctx->pipeline_config_info.viewport.minDepth = 0.0f;
    pctx->pipeline_config_info.viewport.maxDepth = 1.0f;

    pctx->pipeline_config_info.scissor.offset = {0, 0};
    pctx->pipeline_config_info.scissor.extent = {width, height};

    // rasterizationInfo
    pctx->pipeline_config_info.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pctx->pipeline_config_info.rasterizationInfo.depthClampEnable = VK_FALSE;
    pctx->pipeline_config_info.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    pctx->pipeline_config_info.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pctx->pipeline_config_info.rasterizationInfo.lineWidth = 1.0f;
    pctx->pipeline_config_info.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pctx->pipeline_config_info.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pctx->pipeline_config_info.rasterizationInfo.depthBiasEnable = VK_FALSE;
    pctx->pipeline_config_info.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    pctx->pipeline_config_info.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    pctx->pipeline_config_info.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
 
    pctx->pipeline_config_info.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pctx->pipeline_config_info.multisampleInfo.sampleShadingEnable = VK_FALSE;
    pctx->pipeline_config_info.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pctx->pipeline_config_info.multisampleInfo.minSampleShading = 1.0f;           // Optional
    pctx->pipeline_config_info.multisampleInfo.pSampleMask = nullptr;             // Optional
    pctx->pipeline_config_info.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    pctx->pipeline_config_info.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
    
    pctx->pipeline_config_info.colorBlendAttachment.blendEnable = VK_FALSE;
    pctx->pipeline_config_info.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    pctx->pipeline_config_info.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    pctx->pipeline_config_info.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    pctx->pipeline_config_info.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    pctx->pipeline_config_info.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    pctx->pipeline_config_info.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;      
    pctx->pipeline_config_info.colorBlendAttachment.colorWriteMask =  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;

    pctx->pipeline_config_info.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pctx->pipeline_config_info.colorBlendInfo.logicOpEnable = VK_FALSE;
    pctx->pipeline_config_info.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    pctx->pipeline_config_info.colorBlendInfo.attachmentCount = 1;
    pctx->pipeline_config_info.colorBlendInfo.pAttachments = &pctx->pipeline_config_info.colorBlendAttachment;
    pctx->pipeline_config_info.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    pctx->pipeline_config_info.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    pctx->pipeline_config_info.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    pctx->pipeline_config_info.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
    std::cout << pctx->pipeline_config_info.colorBlendInfo.pAttachments;
 
    pctx->pipeline_config_info.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pctx->pipeline_config_info.depthStencilInfo.depthTestEnable = VK_TRUE;
    pctx->pipeline_config_info.depthStencilInfo.depthWriteEnable = VK_TRUE;
    pctx->pipeline_config_info.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    pctx->pipeline_config_info.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    pctx->pipeline_config_info.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
    pctx->pipeline_config_info.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    pctx->pipeline_config_info.depthStencilInfo.stencilTestEnable = VK_FALSE;
    pctx->pipeline_config_info.depthStencilInfo.front = {};  // Optional
    pctx->pipeline_config_info.depthStencilInfo.back = {};   // Optional
    
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
    
    if (pctx->pipeline_config_info.pipelineLayout == VK_NULL_HANDLE) std::cout << "Cannot create graphics pipeline: No pipelineLayout provided in configInfo \n"; 
    if (pctx->pipeline_config_info.renderPass == VK_NULL_HANDLE) std::cout << "Cannot create graphics pipeline: No renderPass provided in configInfo \n"; 

    // Create Shader Modules
    auto vertCode = readFile(Config::get().shader_path() + "simple_shader_vert.spv");
    createShaderModule(vertCode, pctx->vert_shader_module);
    auto fragCode = readFile(Config::get().shader_path() + "simple_shader_frag.spv");
    createShaderModule(fragCode, pctx->frag_shader_module);


    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = pctx->vert_shader_module;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = pctx->frag_shader_module;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    auto bindingDescriptions = SeModel::Vertex::getBindingDescriptions();
    auto attributeDescriptions = SeModel::Vertex::getAttributeDescriptions();
    
    // VkPipelineVertexInputStateCreateInfo
    pctx->pipeline_config_info.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pctx->pipeline_config_info.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    pctx->pipeline_config_info.vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    pctx->pipeline_config_info.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    pctx->pipeline_config_info.vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    


    // VkPipelineViewportStateCreateInfo 
    pctx->pipeline_config_info.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pctx->pipeline_config_info.viewportInfo.viewportCount = 1;
    pctx->pipeline_config_info.viewportInfo.pViewports = &pctx->pipeline_config_info.viewport;
    pctx->pipeline_config_info.viewportInfo.scissorCount = 1;
    pctx->pipeline_config_info.viewportInfo.pScissors = &pctx->pipeline_config_info.scissor;

    // VkGraphicsPipelineCreateInf 
    pctx->pipeline_config_info.graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pctx->pipeline_config_info.graphicsPipelineInfo.pNext = nullptr;
    pctx->pipeline_config_info.graphicsPipelineInfo.flags = 0;
    pctx->pipeline_config_info.graphicsPipelineInfo.stageCount = 2;
    pctx->pipeline_config_info.graphicsPipelineInfo.pStages = shaderStages;
    pctx->pipeline_config_info.graphicsPipelineInfo.pVertexInputState = &pctx->pipeline_config_info.vertexInputInfo;
    pctx->pipeline_config_info.graphicsPipelineInfo.pInputAssemblyState = &pctx->pipeline_config_info.inputAssemblyInfo;
    pctx->pipeline_config_info.graphicsPipelineInfo.pTessellationState = nullptr;
    pctx->pipeline_config_info.graphicsPipelineInfo.pViewportState = &pctx->pipeline_config_info.viewportInfo;
    pctx->pipeline_config_info.graphicsPipelineInfo.pRasterizationState = &pctx->pipeline_config_info.rasterizationInfo;
    pctx->pipeline_config_info.graphicsPipelineInfo.pMultisampleState = &pctx->pipeline_config_info.multisampleInfo;
    pctx->pipeline_config_info.graphicsPipelineInfo.pDepthStencilState = &pctx->pipeline_config_info.depthStencilInfo;
    pctx->pipeline_config_info.graphicsPipelineInfo.pColorBlendState = &pctx->pipeline_config_info.colorBlendInfo;
    pctx->pipeline_config_info.graphicsPipelineInfo.pDynamicState = nullptr;
    pctx->pipeline_config_info.graphicsPipelineInfo.layout = pctx->pipeline_config_info.pipelineLayout;
    pctx->pipeline_config_info.graphicsPipelineInfo.renderPass = pctx->render_pass;
    pctx->pipeline_config_info.graphicsPipelineInfo.subpass = pctx->pipeline_config_info.subpass;
    pctx->pipeline_config_info.graphicsPipelineInfo.basePipelineIndex = -1;
    pctx->pipeline_config_info.graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkResult result = vkCreateGraphicsPipelines(pctx->device, VK_NULL_HANDLE, 1, &pctx->pipeline_config_info.graphicsPipelineInfo, nullptr, &pctx->pipeline_graphics);
    if (result != VK_SUCCESS)
    {
        std::cout << "Failed to create graphics pipeline: " << result << std::endl;
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    
    
}

void SePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule &shaderModule)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(pctx->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        std::cout << "Failed to create shader module" << std::endl;
        throw std::runtime_error("Failed to create shader module");
    }
}
}

