#pragma once

#include "my_camera.hpp"
#include "frame_info.hpp"
#include "my_engine_device.hpp"
#include "my_pipeline.hpp"
#include "my_game_object.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class SimpleRenderSystem {
 public:

  SimpleRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(FrameInfo& frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  VulkanDevice& device;
  std::unique_ptr<MyPipeline> pipeline;
  VkPipelineLayout pipelineLayout;
};
};
 // namespace lve
