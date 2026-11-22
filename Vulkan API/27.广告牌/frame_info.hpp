#pragma once

#include "my_camera.hpp"

#include <vulkan/vulkan.h>
#include "my_game_object.hpp"

namespace lve {
struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    MyCamera camera;
    VkDescriptorSet globalDescriptorSet;
    GameObject::Map &gameObjects;
};
};