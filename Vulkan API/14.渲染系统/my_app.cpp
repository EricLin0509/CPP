#include "my_app.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>

namespace lve {
FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::run() {
  SimpleRenderSystem renderSystem{device, renderer.getRenderPass()};

  while (!window.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = renderer.beginFrame()) {
      renderer.beginSwapChainRenderPass(commandBuffer);
      renderSystem.renderGameObjects(commandBuffer, gameObjects);
      renderer.endSwapChainRenderPass(commandBuffer);
      renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(device.device());
}

void FirstApp::loadGameObjects()
{
  std::vector<MyModel::Vertex> vertices {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  auto model = std::make_shared<MyModel>(device, vertices);

  auto triangle = GameObject::createGameObject();
  triangle.model = model;
  triangle.color = {0.1f, 0.8f, 0.1f};
  triangle.transform2d.translation.x = .2f;
  triangle.transform2d.scale = {2.f, .5f};
  triangle.transform2d.rotation = .25f * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

}  // namespace lve