#include "vulkan_window.hpp"
#include "my_pipeline.hpp"
#include "my_engine_device.hpp"
#include "my_engine_swap_chain.hpp"

#include <memory>
#include <vector>

static constexpr int WINDOW_WIDTH = 800;
static constexpr int WINDOW_HEIGHT = 600;

using namespace lve;

void createPipelineLayout(VulkanDevice &device, VkPipelineLayout &pipelineLayout)
{
    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 0;
    layoutCreateInfo.pSetLayouts = nullptr;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device.device(), &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void createPipeline(VulkanSwapChain &swapChain, VkPipelineLayout &pipelineLayout, VulkanDevice &device, std::unique_ptr<MyPipeline> &pipeline)
{
    auto pipelineConfig = MyPipeline::defaultConfigInfo(swapChain.width(), swapChain.height());
    pipelineConfig.renderPass = swapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    pipeline = std::make_unique<MyPipeline>(
        device,
        pipelineConfig,
        "simple_shader.vert.spv",
        "simple_shader.frag.spv");
}

int main(void) {
    VulkanWindow window(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Vulkan!");
    VulkanDevice device{window};
    VulkanSwapChain swapChain{device, window.getExtent()};
    std::unique_ptr<MyPipeline> pipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffers;

    createPipelineLayout(device, pipelineLayout);
    createPipeline(swapChain, pipelineLayout, device, pipeline);

    while (!window.shouldClose())
    {
        glfwPollEvents();
    }

    return 0;
}