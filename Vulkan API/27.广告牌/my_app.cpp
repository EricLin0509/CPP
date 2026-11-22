#include "my_app.hpp"

#include "my_camera.hpp"
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
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
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
  glm::vec3 lightPosition{-1.f};
  alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity, 
  // vec4 lightColor = (r, g, b, intensity) => (r * intensity, g * intensity, b * intensity)
};

FirstApp::FirstApp() { 
  globalPool = MyDescriptorPool::Builder(device)
    .setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
    .build();

  loadGameObjects();
}

FirstApp::~FirstApp() { }

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

  auto globalSetLayout = MyDescriptorSetLayout::Builder(device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    MyDescriptorWriter(*globalSetLayout, *globalPool)
      .writeBuffer(0, &bufferInfo)
      .build(globalDescriptorSets[i]);
   }

  SimpleRenderSystem renderSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
  PointLightSystem pointLightSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};

  MyCamera camera{};

  auto viewerObject = GameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
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
        camera,
        globalDescriptorSets[frameIndex],
        gameObjects,
      };

      // Update
      GlobalUbo ubo{};
      ubo.projection = camera.getProjectionMatrix();
      ubo.view = camera.getViewMatrix();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      renderer.beginSwapChainRenderPass(commandBuffer);
      renderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);
      renderer.endSwapChainRenderPass(commandBuffer);
      renderer.endFrame();
    }
  }

  vkDeviceWaitIdle(device.device());
}

void FirstApp::loadGameObjects()
{
  std::shared_ptr<MyModel> model = MyModel::createModel(device, "models/smooth_vase.obj");
  auto smoothVase = GameObject::createGameObject();
  smoothVase.model = model;
  // cube.transform.translation = {.0f, .0f, .5f}
  smoothVase.transform.translation = {.0f, .5f, .0f};
  smoothVase.transform.scale = glm::vec3(3.f);

  gameObjects.emplace(smoothVase.getID(), std::move(smoothVase));

  model = MyModel::createModel(device, "models/quad.obj");
  auto floor = GameObject::createGameObject();
  floor.model = model;
  // cube.transform.translation = {.0f, .0f, .5f}
  floor.transform.translation = {.0f, .5f, .0f};
  floor.transform.scale = glm::vec3(3.f);

  gameObjects.emplace(floor.getID(), std::move(floor));
}

}  // namespace lve