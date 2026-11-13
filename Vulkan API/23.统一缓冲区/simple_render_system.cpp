#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>

namespace lve {
struct SimplePushConstantsData {
  glm::mat4 transform{1.f};
  glm::mat4 normalMatrix{1.f};
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

void SimpleRenderSystem::renderGameObjects(
    FrameInfo& frameInfo,
    std::vector<GameObject>& gameObjects)
{
  pipeline->bind(frameInfo.commandBuffer);

  auto projectionView = frameInfo.camera.getProjectionMatrix() * frameInfo.camera.getViewMatrix();

  for (auto& obj : gameObjects) {
    SimplePushConstantsData push = {};
    auto normalMatrix = obj.transform.mat4();
    push.transform = projectionView * normalMatrix;
    push.normalMatrix = normalMatrix;

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