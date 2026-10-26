#pragma once

#include "my_engine_device.hpp"
#include "my_pipeline.hpp"
#include "my_engine_swap_chain.hpp"
#include "my_model.hpp"
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
  void loadModels();
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void recreateSwapChain();
  void recordCommandBuffers(int imageIndex);

  VulkanWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
  VulkanDevice device{window};
  std::unique_ptr<VulkanSwapChain> swapchain;
  std::unique_ptr<MyPipeline> pipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
  std::unique_ptr<MyModel> model;
};
};
 // namespace lve
