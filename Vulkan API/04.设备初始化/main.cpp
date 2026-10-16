#include "vulkan_window.hpp"
#include "my_pipeline.hpp"

static constexpr int WINDOW_WIDTH = 800;
static constexpr int WINDOW_HEIGHT = 600;

using namespace lve;

int main(void) {
    VulkanWindow window(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Vulkan!");
    MyPipeline pipeline("simple_shader.vert.spv", "simple_shader.frag.spv");

    while (!window.shouldClose())
    {
        glfwPollEvents();
    }

    return 0;
}
