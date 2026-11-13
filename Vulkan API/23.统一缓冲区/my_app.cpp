#include "my_app.hpp"

#include "my_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_control.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>

namespace lve {
struct GlobalUbo {
  glm::mat4 ProjectionMatrix{1.f};
  glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
};

FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::run() {
  std::vector<std::unique_ptr<MyBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<MyBuffer>(
      device,
      sizeof(GlobalUbo),
      1,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();

  }

  SimpleRenderSystem renderSystem{device, renderer.getRenderPass()};
  MyCamera camera{};

  auto viewerObject = GameObject::createGameObject();
  KeyboardControl cameraControl{};

  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!window.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraControl.moveInPlaneXZ(window.getWindow(), deltaTime, viewerObject);
    camera.setViewPosition(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = renderer.getAspectRatio();
    // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

    if (auto commandBuffer = renderer.beginFrame()) {
      int frameIndex = renderer.getCurrentFrameIndex();
      FrameInfo frameInfo{
        frameIndex,
        deltaTime,
        commandBuffer,
        camera};

      // Update
      GlobalUbo ubo{};
      ubo.ProjectionMatrix = camera.getProjectionMatrix() * camera.getViewMatrix();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      renderer.beginSwapChainRenderPass(commandBuffer);
      renderSystem.renderGameObjects(frameInfo, gameObjects);
      renderer.endSwapChainRenderPass(commandBuffer);
      renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(device.device());
}

void FirstApp::loadGameObjects()
{
  std::shared_ptr<MyModel> model = MyModel::createModel(device, "models/smooth_vase.obj");
  auto gameObject = GameObject::createGameObject();
  gameObject.model = model;
  // cube.transform.translation = {.0f, .0f, .5f}
  gameObject.transform.translation = {.0f, .5f, 2.5f};
  gameObject.transform.scale = glm::vec3(3.f);

  gameObjects.push_back(std::move(gameObject));
}

}  // namespace lve