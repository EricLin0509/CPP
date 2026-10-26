# 交换链重建 & 动态视口

不幸的是，在 Vulkan 中改变窗口大小并不能简单的修改 `glfwWindowHint` 中的 `GLFW_RESIZABLE` 选项

```cpp
glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
```

如果尝试改变窗口大小，应用会直接崩溃

```bash
validation layer: Cannot call vkDestroyPipeline on VkPipeline that is currently in use by a command buffer
```

主要是因为 `VK_ERROR_OUT_OF_DATE_KHR` 错误，交换链已过期

这时就需要交换链重建 (Swapchain Recreation) 了

## 交换链重建

交换链重建 (Swapchain Recreation) 是在窗口大小改变时，需要重新创建交换链 (Swapchain) 的过程

### 设置窗口大小改变回调

用于通知应用窗口大小改变，需要重建交换链

```cpp
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
```

```cpp
void VulkanWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto lveWindow = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
    lveWindow->framebufferResized = true;
    lveWindow->width = width;
    lveWindow->height = height;
}
```

- 注册 GLFW 帧缓冲区大小回调 `glfwSetFramebufferSizeCallback`，当窗口大小改变时，设置 `framebufferResized = true` 并更新 `width` / `height`
- `glfwSetWindowUserPointer` 将 `this` 指针绑定到 GLFW 窗口，使静态回调函数能访问 `VulkanWindow` 实例

```cpp
bool isFramebufferResized() { return framebufferResized; }
void resetFramebufferResized() { framebufferResized = false; }
```

- `isFramebufferResized()`：查询窗口是否被调整过大小
- `resetFramebufferResized()`：重建交换链后重置标志

### 复用旧交换链资源

由于交换链重建时需要销毁旧交换链，创建新的交换链，所以需要保存旧交换链的资源

```cpp
VulkanSwapChain(VulkanDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapChain> previous);
```

- 新增带 `previous` 参数的构造函数，接收旧交换链用于重建
- `oldSwapChain` 成员保存旧交换链的 `shared_ptr`，在创建新交换链时传递给 `VkSwapchainCreateInfoKHR::oldSwapchain`，让驱动复用旧交换链的资源

```cpp
createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;
```

- `oldSwapchain`：告诉驱动这是从旧交换链重建而来，驱动可以在旧交换链仍在使用时就开始创建新交换链，提高效率

### 重建交换链

```cpp
std::unique_ptr<VulkanSwapChain> swapchain;
```

- 交换链从栈对象变为 `unique_ptr`，因为重建时需要销毁旧交换链并创建新的

```cpp
void recreateSwapChain();
void freeCommandBuffers();
```

#### 重建交换链函数

重建交换链需要以下几步

1. 等待 GPU 完成所有工作，否则会导致死锁
2. 创建新的交换链
    - 如果没有旧交换链，直接创建新的交换链
    - 如果有旧交换链，通过 `oldSwapchain` 复用资源，创建新的交换链
3. 如果新交换链的图像数量变了，需要重新分配命令缓冲区
4. 重新创建渲染管线

```cpp
void FirstApp::recreateSwapChain() {
  auto extent = window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = window.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(device.device());
  
  if (swapchain == nullptr) {
    swapchain = std::make_unique<VulkanSwapChain>(device, extent);
  } else {
    swapchain = std::make_unique<VulkanSwapChain>(device, extent, std::move(swapchain));
    if (swapchain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  createPipeline();
}
```

- `while (extent.width == 0 || extent.height == 0)`：窗口最小化时宽高为 0，此时挂起等待 (`glfwWaitEvents`)，避免创建无效交换链
- `vkDeviceWaitIdle`：等待 GPU 完成所有工作后再重建，防止资源仍在使用时被销毁
- `std::move(swapchain)`：将旧交换链传递给新交换链的构造函数，驱动通过 `oldSwapchain` 复用资源
- 重建后需要重新创建管线 (`createPipeline`)，因为渲染通道等依赖交换链的格式和大小
- 如果新交换链的图像数量变了，需要重新分配命令缓冲区

> **\[补充\]**：`vkDeviceWaitIdle` 是一个**全局同步点**，它会阻塞 CPU 直到 GPU 队列完全空闲。这在窗口拖动 (高频触发) 时会导致明显的卡顿。工业级引擎为了极致流畅，会结合**信号量 (Semaphore) 和栅栏 (Fence)**，或者使用 `oldSwapchain` 延迟销毁旧资源，从而实现**不阻塞 CPU 的平滑重建**。对于初学者，用 `vkDeviceWaitIdle` 是最安全、最不容易写挂的做法

#### 释放旧缓冲区函数

```cpp
void FirstApp::freeCommandBuffers() {
  vkFreeCommandBuffers(device.device(),
  device.getCommandPool(),
  static_cast<uint32_t>(commandBuffers.size()),
  commandBuffers.data());
  
  commandBuffers.clear();
}
```

- 重建交换链前释放旧命令缓冲区，避免资源泄漏

### 处理 `VK_ERROR_OUT_OF_DATE_KHR` 错误

这个错误通常发生在 `vkQueuePresentKHR` 调用后，交换链已与表面不再兼容 (窗口大小改变)，必须重建交换链

```cpp
void FirstApp::drawFrame() {
  ...
  auto result = swapchain->acquireNextImage(&imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }
  ...
  result = swapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.isFramebufferResized()) {
    window.resetFramebufferResized();
    recreateSwapChain();
    return;
  }
  ...
}
```

- `VK_ERROR_OUT_OF_DATE_KHR`：交换链已与表面不再兼容 (窗口大小改变)，必须重建
- `VK_SUBOPTIMAL_KHR`：交换链仍可使用但不是最优，通常也选择重建
- `window.isFramebufferResized()`：即使 `vkQueuePresentKHR` 返回成功，如果 GLFW 检测到窗口大小变化，也需要重建

但这样在窗口大小改变时，视口不会发生任何变化，有什么办法实现平滑缩放呢？

这时就需要使用动态视口 (Dynamic Viewport) 来实现平滑缩放

## 动态视口

动态视口 (Dynamic Viewport) 是在渲染过程中，根据窗口大小动态调整视口 (Viewport) 的大小和位置的过程，以适应窗口大小的改变

### 添加动态状态配置

动态状态 (Dynamic State) 是 Vulkan 提供的一种机制，用于在渲染过程中动态调整管线状态，如视口、裁剪矩形、混合状态等

```cpp
struct PipelineConfigInfo {
  ...
  VkPipelineViewportStateCreateInfo viewportStateInfo;
  std::vector<VkDynamicState> dynamicStates;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  ...
};
```

- 之前 `PipelineConfigInfo` 包含固定的 `VkViewport viewport` 和 `VkRect2D scissor`，管线创建时视口/裁剪矩形就写死了
- 现在改为 `viewportStateInfo` + `dynamicStates` + `dynamicStateInfo`，视口和裁剪矩形在每帧录制命令时动态设置

### 初始化渲染管线配置

由于状态可以发生改变，所以需要在每帧录制命令时动态设置视口和裁剪矩形

```cpp
static void defaultConfigInfo(PipelineConfigInfo& configInfo);
```

- `PipelineConfigInfo` 内部包含指向自身成员的指针 (如 `pAttachments` 指向 `colorBlendAttachment`)
    - 如果按值返回，C++ 的拷贝/移动机制会导致这些指针依然指向原来的临时对象，一旦临时对象销毁，指针就会悬空
    - 为了避免这种深层的生命周期问题，采用引用传参，确保结构体在外部原地构建，保证内部指针永远指向合法的内存

```cpp
configInfo.viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
configInfo.viewportStateInfo.viewportCount = 1;
configInfo.viewportStateInfo.pViewports = nullptr;
configInfo.viewportStateInfo.scissorCount = 1;
configInfo.viewportStateInfo.pScissors = nullptr;
```

- `pViewports = nullptr` / `pScissors = nullptr`：因为视口和裁剪矩形是动态状态，管线创建时不需要提供具体值

```cpp
configInfo.dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStates.data();
configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStates.size());
configInfo.dynamicStateInfo.flags = 0;
```

- 声明 `VK_DYNAMIC_STATE_VIEWPORT` 和 `VK_DYNAMIC_STATE_SCISSOR` 为动态状态
- 管线创建时将这些动态状态信息传递给 `VkGraphicsPipelineCreateInfo::pDynamicState`

### 动态设置视口和裁剪矩形

```cpp
VkViewport viewport{};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
VkRect2D scissor{{0, 0}, swapchain->getSwapChainExtent()};
vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);
```

- 每帧录制命令时，从交换链的当前 `extent` 动态设置视口和裁剪矩形
- 窗口大小改变 → 交换链重建 → `swapchain->getSwapChainExtent()` 返回新大小 → 视口自动适配
- `vkCmdSetViewport` / `vkCmdSetScissor`：在命令缓冲区中动态设置视口和裁剪矩形，替代管线创建时的固定值
