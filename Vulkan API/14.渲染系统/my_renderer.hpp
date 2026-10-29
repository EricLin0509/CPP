#pragma once

#include "my_engine_device.hpp"
#include "my_engine_swap_chain.hpp"
#include "vulkan_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace lve {
class MyRenderer {
 public:

  MyRenderer(VulkanWindow& window, VulkanDevice& device);
  ~MyRenderer();

  MyRenderer(const MyRenderer &) = delete;
  MyRenderer &operator=(const MyRenderer &) = delete;

	VkRenderPass getRenderPass() const { return swapchain->getRenderPass(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
		assert(isFrameInProgress() && "Cannot get current command buffer when no frame is in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getCurrentFrameIndex() const {
    assert(isFrameStarted && "Cannot get current frame index when no frame is in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();

  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

  private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  VulkanWindow& window;
  VulkanDevice& device;
  std::unique_ptr<VulkanSwapChain> swapchain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex;
  bool isFrameStarted = false;
};
};
 // namespace lve
