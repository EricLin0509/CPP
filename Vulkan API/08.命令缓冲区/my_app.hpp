#pragma once

#include "my_engine_device.hpp"
#include "my_pipeline.hpp"
#include "my_engine_swap_chain.hpp"
#include "vulkan_window.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();

  VulkanWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
  VulkanDevice device{window};
  VulkanSwapChain swapchain{device, window.getExtent()};
  std::unique_ptr<MyPipeline> pipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
};
};
 // namespace lve
