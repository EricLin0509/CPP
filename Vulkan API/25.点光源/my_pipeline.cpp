#include "my_pipeline.hpp"
#include "my_model.hpp"

#include <iostream>
#include <fstream>
#include <cassert>

using namespace lve;

MyPipeline::MyPipeline(
            VulkanDevice& device,
            const PipelineConfigInfo& configInfo,
            const char* vertShaderPath,
            const char* fragShaderPath) : device{device}
{
    createPipeline(vertShaderPath, fragShaderPath, configInfo);
}

MyPipeline::~MyPipeline()
{
    vkDestroyPipeline(device.device(), pipeline, nullptr);
    vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
    vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
}

std::vector<char> MyPipeline::readShaderFile(const char* path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void MyPipeline::createPipeline(const char* vertShaderPath, const char* fragShaderPath, const PipelineConfigInfo& configInfo)
{
    assert(configInfo.pipelineLayout != nullptr && "Pipeline layout is not provided");
    assert(configInfo.renderPass != nullptr && "Render pass is not provided");

    /* 读取着色器文件 */
    auto vertCode = readShaderFile(vertShaderPath);
    auto fragCode = readShaderFile(fragShaderPath);

    std::cout << "Vertex shader code size: " << vertCode.size() << " " << fragCode.size() << "\n";
    std::cout << "Fragment shader code size: " << fragCode.size() << "\n";

    createShaderModule(vertCode, &vertShaderModule);
    createShaderModule(fragCode, &fragShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[2];

    /* Initialize shader stages and module explicitly */
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;

    for (uint8_t i = 0; i < 2; i++)
    {
        shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[i].pName = "main";
        shaderStages[i].pNext = nullptr;
        shaderStages[i].flags = 0;
        shaderStages[i].pSpecializationInfo = nullptr;
    }

    auto bindingDescriptions = MyModel::Vertex::getBindingDescriptions();
    auto attributeDescriptions = MyModel::Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    pipelineInfo.pViewportState = &configInfo.viewportStateInfo;
    pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

    pipelineInfo.layout = configInfo.pipelineLayout;
    pipelineInfo.renderPass = configInfo.renderPass;
    pipelineInfo.subpass = configInfo.subpass;
    
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = nullptr;

    if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
}

void MyPipeline::createShaderModule(const std::vector<char> bin_code, VkShaderModule *shaderModule)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = bin_code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(bin_code.data());

    if (vkCreateShaderModule(device.device(), &shaderModuleCreateInfo, nullptr, shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }
}

void MyPipeline::defaultConfigInfo(PipelineConfigInfo& configInfo)
{
  configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
 
  configInfo.viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  configInfo.viewportStateInfo.viewportCount = 1;
  configInfo.viewportStateInfo.pViewports = nullptr;
  configInfo.viewportStateInfo.scissorCount = 1;
  configInfo.viewportStateInfo.pScissors = nullptr;
 
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
 
  configInfo.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
  configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
  configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
  configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
  configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
 
  configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
  configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
  configInfo.colorBlendInfo.attachmentCount = 1;
  configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
  configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
  configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
  configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
  configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
 
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

  configInfo.dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStates.data();
  configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStates.size());
  configInfo.dynamicStateInfo.flags = 0;
}

void MyPipeline::bind(VkCommandBuffer commandBuffer) {
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
