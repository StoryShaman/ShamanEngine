﻿#include "SePipeline.h"

#include <cassert>
#include <Config.h>
#include <fstream>
#include <ios>
#include <iostream>

#include "vulkancontext.h"

namespace SE {
SePipeline::SePipeline(VulkanContext* inctx)
{
    ctx = inctx;
}

SePipeline::~SePipeline()
{
    vkDestroyShaderModule(ctx->device, ctx->vert_shader_module, nullptr);
    vkDestroyShaderModule(ctx->device, ctx->frag_shader_module, nullptr);
    vkDestroyPipeline(ctx->device, ctx->pipeline, nullptr);
}

void SePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
{
    PipelineConfigInfo configInfo{};
    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.viewport.x = 0.0f;
    configInfo.viewport.y = 0.0f;
    configInfo.viewport.width = static_cast<float>(width);
    configInfo.viewport.height = static_cast<float>(height);
    configInfo.viewport.minDepth = 0.0f;
    configInfo.viewport.maxDepth = 1.0f;

    configInfo.scissor.offset = {0, 0};
    configInfo.scissor.extent = {width, height};



    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
 
    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
    configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
    
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;      
    configInfo.colorBlendAttachment.colorWriteMask =  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;

    configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
    std::cout << configInfo.colorBlendInfo.pAttachments;
 
    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {};  // Optional
    configInfo.depthStencilInfo.back = {};   // Optional

    
    ctx->pipeline_config_info = configInfo;
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
    auto vertCode = readFile(Config::get().shader_path() + "simple_shader_vert.spv");
    auto fragCode = readFile(Config::get().shader_path() + "simple_shader_frag.spv");
    std::cout << Config::get().shader_path() + "simple_shader_vert.spv" + "Vertex Shader Code Size" << vertCode.size() << std::endl;
    std::cout << Config::get().shader_path() + "simple_shader_frag.spv" + "Fragment Shader Code Size" << fragCode.size() << std::endl;
    
    if (ctx->pipeline_config_info.pipelineLayout == VK_NULL_HANDLE) std::cout << "Cannot create graphics pipeline: No pipelineLayout provided in configInfo \n"; 
    if (ctx->pipeline_config_info.renderPass == VK_NULL_HANDLE) std::cout << "Cannot create graphics pipeline: No renderPass provided in configInfo \n"; 
  

    createShaderModule(vertCode, ctx->vert_shader_module);
    createShaderModule(fragCode, ctx->frag_shader_module);
    
    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = ctx->vert_shader_module;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = ctx->frag_shader_module;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;

    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &ctx->pipeline_config_info.viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &ctx->pipeline_config_info.scissor;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &ctx->pipeline_config_info.inputAssemblyInfo;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &ctx->pipeline_config_info.rasterizationInfo;
    pipelineInfo.pMultisampleState = &ctx->pipeline_config_info.multisampleInfo;
    pipelineInfo.pDepthStencilState = &ctx->pipeline_config_info.depthStencilInfo;
    pipelineInfo.pColorBlendState = &ctx->pipeline_config_info.colorBlendInfo;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = ctx->pipeline_config_info.pipelineLayout;
    pipelineInfo.renderPass = ctx->pipeline_config_info.renderPass;
    pipelineInfo.subpass = ctx->pipeline_config_info.subpass;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    

    VkResult result = vkCreateGraphicsPipelines(
            ctx->device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &ctx->pipeline);
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

    if (vkCreateShaderModule(ctx->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        std::cout << "Failed to create shader module" << std::endl;
        throw std::runtime_error("Failed to create shader module");
    }
}
}

