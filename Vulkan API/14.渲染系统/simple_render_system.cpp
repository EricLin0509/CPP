#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>

namespace lve {
struct SimplePushConstantsData {
  glm::mat2 transform{1.f};
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

SimpleRenderSystem::SimpleRenderSystem(VulkanDevice& device, VkRenderPass renderPass) : device(device) {
  createPipelineLayout();
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }


void SimpleRenderSystem::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantsData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
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

void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects)
{
  pipeline->bind(commandBuffer);

  for (auto& obj : gameObjects) {
    obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.1f, glm::pi<float>());

    SimplePushConstantsData push = {};
    push.offset = obj.transform2d.translation;
    push.color = obj.color;
    push.transform = obj.transform2d.mat2();

    vkCmdPushConstants(
      commandBuffer,
      pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(push),
      &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

}  // namespace lve