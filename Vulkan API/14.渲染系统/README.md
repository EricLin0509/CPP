# 渲染系统

通常情况下，一个渲染引擎只有一个渲染器 (Renderer)，但可以有多个系统 (例如纹理渲染系统、模型渲染系统等)

两者的分工是：

| 角色 | 关心的事 | 例子 |
| :--: | :----- | :--- |
| 渲染器 | 交换链、命令缓冲区、帧的生命周期、渲染通道的开关 | [my_renderer.hpp](my_renderer.hpp) |
| 渲染系统 | 管线与管线布局、某一类对象怎么画 | [simple_render_system.hpp](simple_render_system.hpp) |

渲染器不关心"画什么"，渲染系统不关心"帧怎么推进"，两者通过命令缓冲区与渲染通道连接

## 渲染器 MyRenderer

渲染器把与帧相关的一切收归己有：

```cpp
class MyRenderer {
 public:
  MyRenderer(VulkanWindow& window, VulkanDevice& device);
  ~MyRenderer();

  MyRenderer(const MyRenderer &) = delete;
  MyRenderer &operator=(const MyRenderer &) = delete;

  VkRenderPass getRenderPass() const { return swapchain->getRenderPass(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameInProgress() && "Cannot get current command buffer when no frame is in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getCurrentFrameIndex() const {
    assert(isFrameStarted && "Cannot get current frame index when no frame is in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();

  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  VulkanWindow& window;
  VulkanDevice& device;
  std::unique_ptr<VulkanSwapChain> swapchain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex;
  bool isFrameStarted = false;
};
```

持有物：

- **`swapchain`**：由渲染器独占创建与重建，外界只通过 `getRenderPass()` 拿到渲染通道句柄
- **`commandBuffers`**：数量等于 `VulkanSwapChain::MAX_FRAMES_IN_FLIGHT` (这里是 2)，按"帧"而不是按"交换链图像"分配，因此同一个命令缓冲区会被不同图像反复复用
- **`currentImageIndex`**：本帧要画的交换链图像下标，`beginFrame` 里由 `acquireNextImage` 写入
- **`currentFrameIndex`**：本帧使用哪个命令缓冲区，每帧 `endFrame` 时 `(currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT` 轮转
- **`isFrameStarted`**：帧状态标志，把"帧外调用帧内接口"这种错误在断言中直接暴露出来

构造时先 `recreateSwapChain()` 再 `createCommandBuffers()`，顺序不能反：命令缓冲区本身不依赖交换链，但渲染通道的句柄来自交换链，先有交换链才能保证外部一构造完就能拿到有效的渲染通道

### 帧的生命周期

`beginFrame` 负责"取图像 + 开始录制"，返回可以直接录制命令的命令缓冲区：

```cpp
VkCommandBuffer MyRenderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = swapchain->acquireNextImage(&currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  return commandBuffer;
}
```

- **返回 `nullptr` 表示这一帧不存在**：窗口最小化时交换链尺寸为 0，`acquireNextImage` 返回 `VK_ERROR_OUT_OF_DATE_KHR`，此时只重建交换链并让调用方跳过整帧，而不是抛出异常
- **`VK_SUBOPTIMAL_KHR` 不算失败**：图像仍然可用，只是不再完美匹配表面，先照常画这一帧，等到 `endFrame` 提交时再重建
- **`isFrameStarted = true` 放在取图像成功之后**：取图像失败时不能进入"帧进行中"状态，否则下一帧的断言会误报

`endFrame` 负责"结束录制 + 提交 + 呈现"：

```cpp
void MyRenderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.isFramebufferResized()) {
    window.resetFramebufferResized();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
}
```

- **重建的触发条件集中在提交之后**：`OUT_OF_DATE`、`SUBOPTIMAL`、窗口尺寸变化三种情况都在这里统一处理，`isFramebufferResized()` 标志读完即清，避免重复重建
- **`currentFrameIndex` 只在帧成功结束后轮转**：跳过帧（`beginFrame` 返回 `nullptr`）时它保持不变，保证命令缓冲区的复用节奏与"已提交帧数"一致
- **提交与呈现一步完成**：`submitCommandBuffers` 内部完成等待信号量、提交队列、`vkQueuePresentKHR`，渲染器不需要自己管理同步对象

### 渲染通道的开关

渲染通道的 `VkRenderPassBeginInfo` 需要交换链提供的帧缓冲与范围，所以开关也放在渲染器里：

```cpp
void MyRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass while frame is not in progress");
  assert(
    commandBuffer == getCurrentCommandBuffer() &&
    "Can't begin render pass on command buffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = swapchain->getRenderPass();
  renderPassInfo.framebuffer = swapchain->getFrameBuffer(currentImageIndex);
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, swapchain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
```

要点：

- **断言绑定"本帧的命令缓冲区"**：只有当前帧那条命令缓冲区能开启渲染通道，防止把不同帧的录制内容混到一起
- **清屏值按附件顺序排列**：索引 0 是颜色附件、索引 1 是深度附件，`clearValueCount` 必须与渲染通道的附件数量一致
- **深度清成 1.0**：深度测试默认 `VK_COMPARE_OP_LESS`，清成最大值才能让所有通过深度测试的片元覆盖到画面
- **视口与裁剪矩形每帧动态设置**：交换链重建后尺寸会变，用动态状态（`vkCmdSetViewport` / `vkCmdSetScissor`）就无需为每种尺寸重新创建管线
- **`endSwapChainRenderPass` 只做 `vkCmdEndRenderPass`**：它与 `begin` 成对出现，同样带帧状态断言，之后由 `endFrame` 统一收尾

## 渲染系统 SimpleRenderSystem

一个渲染系统 = 一套管线布局 + 一条管线 + 针对某一类对象的绘制循环：

```cpp
class SimpleRenderSystem {
 public:
  SimpleRenderSystem(VulkanDevice& device, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects);

 private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  VulkanDevice& device;
  std::unique_ptr<MyPipeline> pipeline;
  VkPipelineLayout pipelineLayout;
};
```

**渲染通道由外部传入**：渲染通道属于交换链，而交换链归渲染器所有，所以渲染系统构造时接收 `VkRenderPass`，只负责把自己的管线绑定到这个通道上，不负责创建它

### 管线布局与推送常量

```cpp
void SimpleRenderSystem::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantsData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}
```

- **`stageFlags` 同时包含顶点与片段阶段**：变换与偏移在顶点着色器里用，颜色在片段着色器里用，同一个推送块被两个阶段共享，阶段标志必须覆盖两者
- **`offset = 0`、`size = sizeof(SimplePushConstantsData)`**：一块从 0 开始、覆盖整个结构体的连续区域，推送时的偏移与大小必须与之对应
- **`setLayoutCount = 0`**：没有描述符集（没有 UBO、纹理），所有逐对象数据都走推送常量
- **管线布局归渲染系统持有并负责销毁**：它是渲染系统内部的资源，销毁顺序上必须晚于管线

### 管线创建

```cpp
void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  PipelineConfigInfo pipelineConfigInfo{};
  MyPipeline::defaultConfigInfo(pipelineConfigInfo);
  pipelineConfigInfo.renderPass = renderPass;
  pipelineConfigInfo.pipelineLayout = pipelineLayout;
  pipeline = std::make_unique<MyPipeline>(
      device,
      pipelineConfigInfo,
      "simple_shader.vert.spv",
      "simple_shader.frag.spv");
}
```

- **从默认配置起步再改两项**：`defaultConfigInfo` 填好固定功能阶段的默认值，渲染系统只覆盖与自身相关的 `renderPass` 和 `pipelineLayout`
- **`renderPass` 决定管线的兼容性**：管线在创建时就把渲染通道"焊死"，之后只能在该通道（或与之兼容的通道）内使用，这也是交换链重建时必须比较格式的原因
- **着色器路径由渲染系统决定**：管线对象只负责"读文件 + 建模块 + 组装"，具体用哪对着色器是渲染系统的选择，换一套着色器就是换一个渲染系统

### 绘制循环

```cpp
void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects) {
  pipeline->bind(commandBuffer);

  for (auto& obj : gameObjects) {
    obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.1f, glm::pi<float>());

    SimplePushConstantsData push = {};
    push.offset = obj.transform2d.translation;
    push.color = obj.color;
    push.transform = obj.transform2d.mat2();

    vkCmdPushConstants(
      commandBuffer,
      pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(push),
      &push);

    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}
```

- **参数是 `std::vector<GameObject>&`**：引用而非拷贝，且允许在录制阶段直接修改对象状态（这里的自转就是逐帧累加旋转角）
- **管线只绑定一次**：所有对象共用同一套管线与管线布局，循环里只重复"推送常量 → 绑定模型 → 绘制"
- **逐对象推送常量**：`offset`、`transform`、`color` 每个对象都不同，因此同一个模型能以不同位置、姿态、颜色画多次，无需描述符集
- **渲染系统不接触帧状态**：它只往传进来的命令缓冲区里录命令，不知道当前是第几帧、画到第几张图像

## 交换链格式比较

交换链重建后，新交换链的附件格式可能变化（例如窗口移动到另一块显示器），此时旧的渲染通道与管线就不再兼容，所以交换链缓存自己的格式并提供比较接口：

```cpp
bool compareSwapFormats(const VulkanSwapChain &swapChain) const {
  return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
         swapChain.swapChainImageFormat == swapChainImageFormat;
}
```

`swapChainDepthFormat` 在 `createDepthResources()` 中随深度格式的选取一并记录：

```cpp
void VulkanSwapChain::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();
  swapChainDepthFormat = depthFormat;
  // ...
}
```

重建时的使用方式：

```cpp
void MyRenderer::recreateSwapChain() {
  auto extent = window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = window.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(device.device());

  if (swapchain == nullptr) {
    swapchain = std::make_unique<VulkanSwapChain>(device, extent);
  } else {
    std::shared_ptr<VulkanSwapChain> oldSwapchain = std::move(swapchain);
    swapchain = std::make_unique<VulkanSwapChain>(device, extent, oldSwapchain);

    if (!oldSwapchain->compareSwapFormats(*swapchain.get())) {
      throw std::runtime_error("swap chain formats has changed!");
    }
  }
}
```

- **先等尺寸非零**：窗口最小化时 `extent` 为 `(0, 0)`，`vkCreateSwapchainKHR` 不允许这种尺寸，所以阻塞在 `glfwWaitEvents()` 直到窗口恢复
- **`vkDeviceWaitIdle` 在重建之前**：交换链的图像可能仍被 GPU 使用，必须等设备空闲才能销毁旧交换链
- **首次创建与重建分两条路径**：重建时把旧交换链以 `shared_ptr` 交给新交换链，让 Vulkan 复用底层图像资源，同时保证旧对象在新交换链构造期间不被销毁
- **格式不一致就抛异常**：渲染通道与管线都无法在格式变化后继续使用，直接终止比带着不兼容的管线继续画更安全

## 应用层 FirstApp

应用层只剩"窗口、设备、渲染器、对象列表"四样东西：

```cpp
class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void loadGameObjects();

  VulkanWindow window{WIDTH, HEIGHT, "Hello Vulkan!"};
  VulkanDevice device{window};
  MyRenderer renderer{window, device};

  std::vector<GameObject> gameObjects;
};
```

- **成员声明顺序即初始化顺序**：`window` → `device` → `renderer`，每个都依赖前一个，顺序不能调换；析构时反向进行，`renderer` 先于 `device` 销毁
- **`renderer` 用成员初始化列表构造**：交换链在构造时就建立，应用一启动就有可用的渲染通道
- **对象列表属于应用层**：场景内容由应用决定，渲染器与渲染系统只接收它们

主循环把各层串起来：

```cpp
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
```

- **渲染系统在 `run()` 里构造**：它需要渲染通道，而渲染通道来自已经构造好的 `renderer`，所以只能在这里创建；它比 `renderer` 晚创建、早销毁，符合资源依赖顺序
- **`if (auto commandBuffer = renderer.beginFrame())`**：返回 `nullptr` 时整帧（包括渲染通道与绘制）都被跳过，最小化窗口不会崩溃
- **四步顺序固定**：`beginFrame` → `beginSwapChainRenderPass` → `renderGameObjects` → `endSwapChainRenderPass` → `endFrame`，中间三步都在同一条命令缓冲区内录制
- **渲染系统与渲染器互不知情**：它们只共享一个 `VkCommandBuffer`，新增渲染系统时只需在循环里多插一行调用
- **退出前 `vkDeviceWaitIdle`**：保证所有在途帧都执行完，避免对象析构时销毁仍被 GPU 使用的资源

依赖关系可以概括为一条单向链：

```
VulkanWindow ──> VulkanDevice ──> VulkanSwapChain ──> MyRenderer ──> SimpleRenderSystem
                                                        │
                                                    GameObject / MyModel（数据）
```

上层只向下层索取句柄（`getRenderPass()`、`getCurrentCommandBuffer()`），下层不反向调用上层，因此新增一个渲染系统不需要改动渲染器
