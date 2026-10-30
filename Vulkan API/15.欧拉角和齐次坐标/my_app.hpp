#pragma once

#include "my_engine_device.hpp"
#include "my_game_object.hpp"
#include "my_renderer.hpp"
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
  void loadGameObjects();

  VulkanWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
  VulkanDevice device{window};
  MyRenderer renderer{window, device};

  std::vector<GameObject> gameObjects;
};
};
 // namespace lve
