#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>

namespace lve {
struct SimplePushConstantsData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

SimpleRenderSystem::SimpleRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device(device) {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }


void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantsData);

  std::vector<VkDescriptorSetLayout> setLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
  pipelineLayoutInfo.pSetLayouts = setLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  PipelineConfigInfo pipelineConfigInfo{};
  MyPipeline::defaultConfigInfo(pipelineConfigInfo);
  pipelineConfigInfo.renderPass = renderPass;
  pipelineConfigInfo.pipelineLayout = pipelineLayout;
  pipeline = std::make_unique<MyPipeline>(
      device,
      pipelineConfigInfo,
      "simple_shader.vert.spv",
      "simple_shader.frag.spv");
}

void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo)
{
  pipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
    frameInfo.commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout,
    0,
    1,
    &frameInfo.globalDescriptorSet,
    0,
    nullptr);

  for (auto& kv_pair : frameInfo.gameObjects) {
    auto& obj = kv_pair.second;
    if(obj.model == nullptr) continue;

    SimplePushConstantsData push = {};
    push.modelMatrix = obj.transform.mat4();
    push.normalMatrix = obj.transform.normalMatrix();

    vkCmdPushConstants(
      frameInfo.commandBuffer,
      pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(push),
      &push);
    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

}  // namespace lve