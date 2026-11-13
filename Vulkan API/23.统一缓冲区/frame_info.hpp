#pragma once

#include "my_camera.hpp"

#include <vulkan/vulkan.h>

namespace lve {
struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    MyCamera camera;
};
};