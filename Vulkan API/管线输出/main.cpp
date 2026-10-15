#include "vulkan_window.hpp"
#include "my_pipeline.hpp"
#include "my_engine_device.hpp"

static constexpr int WINDOW_WIDTH = 800;
static constexpr int WINDOW_HEIGHT = 600;

using namespace lve;

int main(void) {
    VulkanWindow window(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Vulkan!");
    VulkanDevice device{window};
    MyPipeline pipeline(
        device,
        MyPipeline::defaultConfigInfo(WINDOW_WIDTH, WINDOW_HEIGHT),
        "simple_shader.vert.spv",
        "simple_shader.frag.spv");

    while (!window.shouldClose())
    {
        glfwPollEvents();
    }

    return 0;
}
