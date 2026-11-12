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
FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::run() {
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
      renderer.beginSwapChainRenderPass(commandBuffer);
      renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
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