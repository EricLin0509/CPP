# 交换链

交换链 (Swapchain) 是一组帧缓冲区，用于将渲染结果呈现到屏幕上

它允许有不同类型的帧缓冲区，例如颜色、深度、模板等

## 缓冲模式

缓冲模式决定了交换链中帧缓冲区的数量和图像呈现的调度策略

它直接影响两个关键问题：

- **画面撕裂**：当 GPU 渲染速度与显示器刷新不同步时，屏幕可能同时显示两帧的不同部分，产生水平撕裂线
- **帧率与延迟**：更多缓冲区可以让 GPU 不必等待 VSync 信号，提高渲染吞吐量，但也会引入额外延迟

选择哪种缓冲模式，本质上是在**撕裂、帧率、延迟**三者之间做权衡

常见缓冲模式有: 
- 双重缓冲
- 三重缓冲

### 双重缓冲区

双重缓冲 (Double Buffering) 使用两个帧缓冲区：**前缓冲区** (Front Buffer) 和**后缓冲区** (Back Buffer)

- **前缓冲区**：当前正在显示的图像
- **后缓冲区**：下一帧正在渲染的图像

工作流程：
1. GPU 在后缓冲区中渲染新一帧
2. 渲染完成后，交换前后缓冲区 (即"交换"操作)
3. 前缓冲区显示新帧，后缓冲区准备下一帧

- 优点
    - 实现简单
    - 内存占用少

- 缺点
    - 如果交换时机与显示器刷新不同步，可能产生**撕裂** (Tearing) 现象——画面上半部分和下半部分来自不同帧

### 三重缓冲区

三重缓冲 (Triple Buffering) 使用三个帧缓冲区

- 一个前缓冲区用于显示
- 两个后缓冲区用于渲染

工作流程：
1. GPU 始终在空闲的后缓冲区中渲染
2. 当垂直同步信号到来时，将最新渲染完成的后缓冲区交换为前缓冲区
3. GPU 无需等待垂直同步，可以立即在另一个后缓冲区继续渲染

- 优点
    - 消除撕裂现象
    - GPU 不需要等待垂直同步信号，渲染效率更高
    - 帧率更平滑

- 缺点
    - 多占用一个帧缓冲区的显存，引入约一帧的额外延迟

## 交换链创建 (createSwapChain)

交换链的创建分为三个选择步骤和一个填写步骤

### 1. 选择表面格式 (chooseSwapSurfaceFormat)

表面格式由**像素格式** (format) 和**颜色空间** (colorSpace) 两个属性组成

```cpp
VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats);
```

```cpp
for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return availableFormat;
    }
}
return availableFormats[0];
```

- 优先选择 `VK_FORMAT_B8G8R8A8_UNORM` + `VK_COLOR_SPACE_SRGB_NONLINEAR_KHR` 组合
- 如果不支持，回退到第一个可用格式

#### 为什么选 UNORM 而不是 SRGB？

虽然我们想要 sRGB 颜色空间，但格式应选 `UNORM` 而非 `SRGB`

这是因为 Vulkan 的 `SRGB` 格式格式转换会在采样时自动进行 gamma 解码，而在存储时自动进行 gamma 编码。对于交换链图像，我们希望帧缓冲中存储的是线性值，由显示器做最终的 gamma 校正，所以用 `UNORM` 格式 + `SRGB_NONLINEAR` 颜色空间的组合

### 2. 选择呈现模式 (chooseSwapPresentMode)

呈现模式决定了交换链图像何时和如何呈现到屏幕

由于 Vulkan 中并没有直接选“双重缓冲还是三重缓冲”的参数，而是引入了一个更底层的概念：呈现模式 (Present Mode)

它由申请的交换链图像数量 (Image Count) 和选择的呈现模式 (Present Mode)，共同决定了最终表现出来的是双缓冲还是三缓冲的效果

```cpp
VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes);
```

```cpp
for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        return availablePresentMode;
    }
}
return VK_PRESENT_MODE_FIFO_KHR;
```

选择优先级：

1. **`VK_PRESENT_MODE_MAILBOX_KHR`** (邮箱模式)：三重缓冲变体，GPU 渲染完新帧后替换队列中已排队的旧帧，而不是阻塞等待。低延迟且无撕裂
2. **`VK_PRESENT_MODE_FIFO_KHR`** (FIFO 模式)：垂直同步，保证可用。图像按顺序排队，在垂直空白期呈现

| 呈现模式 | 等效缓冲 | 撕裂 | 阻塞 GPU | 说明 |
| :------: | :------: | :--: | :------: | :--- |
| `IMMEDIATE` | 双缓冲 | ✓ 有 | ✗ 不阻塞 | 立即呈现，最低延迟但可能撕裂 |
| `FIFO` | 双缓冲 + VSync | ✗ 无 | ✓ 阻塞 | 垂直同步，最稳定但帧率受刷新率限制 |
| `FIFO_RELAXED` | 双缓冲 + 宽松VSync | 可能 | ✗ 不阻塞 | 帧率 ≥ 刷新率时无撕裂，低于时可能撕裂 |
| `MAILBOX` | 三缓冲 | ✗ 无 | ✗ 不阻塞 | 替换而非排队，低延迟无撕裂的最佳选择，但**它极其消耗 GPU 算力** |

### 3. 选择交换链范围 (chooseSwapExtent)

交换链范围即交换链图像的像素尺寸

```cpp
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
```

```cpp
if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
} else {
    VkExtent2D actualExtent = windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
}
```

- 如果 `currentExtent.width` 不是 `uint32_t::max`，说明窗口管理系统严格指定了交换链尺寸，直接使用
- 否则，将窗口尺寸钳制到 `[minImageExtent, maxImageExtent]` 范围内
- 相比裁剪矩形 (Scissor)，它决定的是**整条流水线最终生产出的画布 (画布图像本身) 有多大**

#### 与裁剪矩形 (Scissor) 的区别

| 特征 | 选择交换链范围 (chooseSwapExtent) | 裁剪矩形 (Scissor) |
| :--: | :----------------------------- | :---------------- |
| 所属阶段 | 交换链初始化阶段 (Swapchain Creation) | 图形管线配置阶段 (Fixed-Function Pipeline) |
| 操控对象 | 显存里真实的 `VkImage` 物理图像资源 | GPU 光栅化后的像素片段写入开关 |
| 数据类型 | `VkExtent2D` (只有宽高，无偏移) | `VkRect2D` (包含 offset 偏移和 extent 宽高) |
| 动态改变 | 极难，需要重建整个交换链 (Recreate Swapchain) | 极易，可通过启用管线动态状态 (Dynamic State) 每帧甚至每次绘制直接修改 |

### 4. 填写交换链创建信息

```cpp
VkSwapchainCreateInfoKHR createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
createInfo.surface = device.surface();

createInfo.minImageCount = imageCount;
createInfo.imageFormat = surfaceFormat.format;
createInfo.imageColorSpace = surfaceFormat.colorSpace;
createInfo.imageExtent = extent;
createInfo.imageArrayLayers = 1;
createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
```

- `surface`：交换链关联的窗口表面
- `minImageCount`：交换链中图像的数量，取 `minImageCount + 1` (多一个缓冲区实现双/三缓冲)，但不超过 `maxImageCount`
- `imageArrayLayers`：每个图像的层数，通常为 1 (立体渲染时为 2)
- `imageUsage`：图像用途，`VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT` 表示作为颜色附件

#### 图像共享模式

图像共享模式 (imageSharingMode) 决定了交换链中的那些物理画布图像 (`VkImage`)

到底是如何在不同的硬件队列族 (Queue Families) 之间进行所有权 (Ownership) 流转和访问的

```cpp
QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
} else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
}
```

一共只有两种共享模式：
- **`VK_SHARING_MODE_EXCLUSIVE` (独占模式)**
    - 规则：在任意一个时间点，这张画布图像**只能属于一个队列族**
    - 工作原理：如果图形队列正在往画布里画画，那么呈现队列连看都不准看这张图一眼。当图形队列画完了，如果你想让呈现队列把它拿去显示，必须在代码里显式写下一个**内存所有权屏障 (Ownership Barrier)**，正式下达公文把这张图的所有权从图形队列转移给呈现队列
    - 优点：性能最高 (零驱动开销)。因为驱动程序不需要在底层做任何多队列并发的安全锁和缓存同
    - 缺点：如果两个队列族不同，你必须手动写很多恶心的屏障 (Barrier) 代码来回倒腾所有权，极为繁琐
- **`VK_SHARING_MODE_CONCURRENT` (并发模式)**
    - 规则：这张画布图像**可以同时被多个指定的队列族共同访问**，不需要任何所有权的转移操作
    - 工作原理：你在创建交换链时，提前把一个包含图形队列索引和呈现队列索引的数组 (`queueFamilyIndices[]`) 塞给它。从此以后，这两队列都可以随时对它进行读写，Vulkan 会在底层帮你打好同步的补丁
    - 优点：写代码最省心。你不需要去管谁拿着这张图，两边队列直接调 API 即可
    - 缺点：有轻微的性能损耗。因为显卡为了防止两边冲突，必须在底层做硬件级别的内存同步和缓存刷新，无法发挥显存的最佳带宽

如果选了 `EXCLUSIVE` (独占模式)，且图形队列和呈现队列不是同一个队列族。在画完图后，要把它交给呈现队列显示时，绝对不能直接调用 `vkQueuePresentKHR`

必须在提交命令时，插入一个 `VkImageMemoryBarrier` (内存屏障)，显式地把这张画布的 `queueFamilyIndex` 从“图形队列”改成“呈现队列” (这叫 Release 和 Acquire 操作)

后果：如果忘记转移所有权，验证层会疯狂报错，画面直接黑屏或驱动崩溃。选 `CONCURRENT` (并发模式) 就可以完全避免这个烦恼 (驱动在底层帮你做了安全锁)

#### 其他字段

```cpp
createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
createInfo.presentMode = presentMode;
createInfo.clipped = VK_TRUE;
createInfo.oldSwapchain = VK_NULL_HANDLE;
```

- `preTransform`：对交换链图像的变换 (如旋转)，使用 `currentTransform` 表示不做额外变换
- `compositeAlpha`：与窗口系统的 alpha 混合方式，`OPAQUE` 表示不透明
- `presentMode`：之前选择的呈现模式
- `clipped = VK_TRUE`：裁剪被遮挡的像素，提升性能
- `oldSwapchain`：重建交换链时传入旧交换链，首次创建为 `VK_NULL_HANDLE`
    - 在修改窗口大小时，我们通常需要重建整个交换链。不要把旧交换链直接 `vkDestroySwapchainKHR`，而是把它赋值给新创建信息里的 `createInfo.oldSwapchain`
    - 这样驱动在创建新交换链时，可以直接接管旧交换链的资源 (比如复用显存队列)，并在新交换链创建成功后，自动让旧的退役。这能避免画面在重建瞬间发生闪烁或卡顿

### 5. 获取交换链图像

```cpp
vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
swapChainImages.resize(imageCount);
vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());
```

注意：`minImageCount` 只是**最小值**，实现可能创建更多图像。所以需要两次调用 `vkGetSwapchainImagesKHR`：

1. 第一次传 `nullptr` 获取实际图像数量
2. 第二次获取所有图像句柄

## 图像视图 (createImageViews)

图像视图描述了如何访问交换链图像——以什么格式、什么范围来解读图像数据

```cpp
VkImageViewCreateInfo viewInfo{};
viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
viewInfo.image = swapChainImages[i];
viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
viewInfo.format = swapChainImageFormat;
viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
viewInfo.subresourceRange.baseMipLevel = 0;
viewInfo.subresourceRange.levelCount = 1;
viewInfo.subresourceRange.baseArrayLayer = 0;
viewInfo.subresourceRange.layerCount = 1;
```

- `viewType = VK_IMAGE_VIEW_TYPE_2D`：将图像视为 2D 纹理
- `format`：与交换链图像格式一致
- `aspectMask = VK_IMAGE_ASPECT_COLOR_BIT`：只访问颜色数据
- `baseMipLevel` / `levelCount`：Mip 层级范围，这里只用第 0 层、共 1 层
- `baseArrayLayer` / `layerCount`：数组层范围，这里只用第 0 层、共 1 层

### 为什么需要图像视图？

Vulkan 中的 `VkImage` 只是原始图像数据的"容器"，不携带任何格式和访问方式信息

图像视图相当于一个"解读器"，告诉 GPU："把这个图像当作 2D 纹理、以 B8G8R8A8 格式、只看颜色通道来使用"

同一张 `VkImage` 可以创建多个不同的 `VkImageView`，以不同方式解读同一份数据

## 渲染通道 (createRenderPass)

渲染通道 (Render Pass) 是 Vulkan 中描述一次渲染过程的核心对象

它告诉 GPU：这次渲染要用哪些附件 (颜色、深度)，每个附件在渲染前后处于什么布局，以及子通道之间的执行顺序和同步关系

### 为什么需要渲染通道？

传统图形 API (如 OpenGL)隐式管理渲染状态——绑定帧缓冲区后直接绘制，驱动自动推断同步和布局转换

Vulkan 将这些信息显式化，渲染通道的优势：

- **布局转换**：Vulkan 图像有多种布局 (`UNDEFINED`、`COLOR_ATTACHMENT_OPTIMAL`、`PRESENT_SRC_KHR` 等)，不同布局对 GPU 访问效率不同。渲染通道在附件的 `initialLayout` 和 `finalLayout` 之间自动插入布局转换屏障
- **显式同步**：通过子通道依赖，精确指定子通道之间、子通道与外部操作之间的执行顺序，避免不必要的管线停顿
- **分块渲染优化**：移动端 GPU 采用分块渲染 (Tiled Rendering)，渲染通道让驱动知道哪些附件在同一子通道中使用，从而将颜色/深度保持在高速片上缓存中，避免频繁写回主存

### 渲染通道的组成结构

```
VkRenderPassCreateInfo
  ├── pAttachments[] ──→ [ colorAttachment (索引0), depthAttachment (索引1) ]
  ├── pSubpasses[]   ──→ [ subpass 0: 引用附件0和附件1 ]
  └── pDependencies[]──→ [ subpass 0 对 VK_SUBPASS_EXTERNAL 的依赖 ]
```

- `VkAttachmentDescription` (**附件描述**)：定义每个附件的格式、加载/存储操作、布局转换
- `VkSubpassDescription` (**子通道描述**)：定义一个渲染子步骤，引用哪些附件作为颜色目标、深度目标等
- `VkSubpassDependency` (**子通道依赖**)：定义子通道之间或子通道与外部操作之间的同步关系

### 颜色附件

当片段着色器 (Fragment Shader) 在 GPU 里疯狂计算出每一个像素的 `vec4(R, G, B, A)` 颜色后，这些颜色总得有一个物理地方把它存下来

颜色附件就是保存这些颜色数据的地方

```cpp
VkAttachmentDescription colorAttachment = {};
colorAttachment.format = getSwapChainImageFormat();
colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
```

- `format`：与交换链图像格式一致
- `loadOp` (Load Operation): 渲染开始时应该如何处理附件的内容 (开始时怎么做?)
    - `VK_ATTACHMENT_LOAD_OP_CLEAR`：渲染开始时清空为指定颜色
- `storeOp` (Store Operation): 渲染结束后应该如何处理附件的内容 (结束时怎么做?)
    - `VK_ATTACHMENT_STORE_OP_STORE`：渲染结束后保留内容 (需要呈现到屏幕)
- `initialLayout = VK_IMAGE_LAYOUT_UNDEFINED`：不关心之前的内容 (会被清空)
- `finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`：渲染结束后转换为呈现布局，供交换链呈现
    - 由于颜色数据需要呈现到屏幕，所以可以使用 `PRESENT_SRC_KHR` 布局 (专供屏幕呈现的通用格式)

#### loadOp / storeOp 详解

| 操作 | `LOAD_OP_CLEAR` | `LOAD_OP_LOAD` | `LOAD_OP_DONT_CARE` |
| :--: | :--- | :--- | :--- |
| 含义 | 渲染开始时清空为指定值 | 保留上一帧的内容 | 不关心之前的内容 |
| 开销 | 低 (无需读取) | 高 (需从内存加载) | 最低 (驱动可自由优化) |
| 典型场景 | 每帧重新渲染的颜色/深度 | 多通道渲染中保留上一通道结果 | 一次性使用的中间附件 |

颜色附件选 `CLEAR` + `STORE`：每帧清空重绘，渲染后保留 (要呈现到屏幕)

深度附件选 `CLEAR` + `DONT_CARE`：每帧清空重绘，渲染后不保留 (下一帧会重新清空，无需浪费带宽写回内存)

#### 图像布局转换

Vulkan 图像在不同阶段需要处于不同的布局，布局影响 GPU 的访问方式：

```
颜色附件：

  VK_IMAGE_LAYOUT_UNDEFINED          渲染通道开始
          │                               │
          │  自动插入布局转换屏障           │
          ▼                               ▼
  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL    渲染通道结束
                                              │
                                              │  自动插入布局转换屏障
                                              ▼
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
```

- `UNDEFINED` → `COLOR_ATTACHMENT_OPTIMAL`：GPU 将图像重新排列为颜色写入最优的内存布局
- `COLOR_ATTACHMENT_OPTIMAL` → `PRESENT_SRC_KHR`：转换为交换链呈现所需的布局

这些转换由 Vulkan 在渲染通道边界自动执行，开发者只需指定 `initialLayout` 和 `finalLayout`

### 深度附件

与颜色附近类似。当片段着色器 (Fragment Shader) 在 GPU 里疯狂计算出每一个像素的深度后，这些深度总得有一个物理地方把它存下来

深度附件就是保存这些深度数据的地方

```cpp
VkAttachmentDescription depthAttachment{};
depthAttachment.format = findDepthFormat();
depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
```

- 这里 `storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE`，因为深度数据这里只是用于画三角形，渲染完成后它就彻底没用了
    - 它在渲染完最后一个三角形后，直接在芯片内部一枪把高速缓存里的深度数据全部抹掉，任何数据都懒得往物理显存里写
    - 下一帧开始时，直接调用 `LOAD_OP_CLEAR` 重新当成空白草稿纸使用
- `finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL`：转换为深度/模板附件最优布局
    - 由于深度数据不需要呈现到屏幕，所以可以使用 `DEPTH_STENCIL_ATTACHMENT_OPTIMAL` 布局 (最适合 GPU 读写的专属深度格式)

#### 深度格式选择

```cpp
VkFormat VulkanSwapChain::findDepthFormat() {
    return device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
```

按优先级尝试三种深度格式：

1. `VK_FORMAT_D32_SFLOAT`：32 位浮点深度，无模板
2. `VK_FORMAT_D32_SFLOAT_S8_UINT`：32 位浮点深度 + 8 位模板
3. `VK_FORMAT_D24_UNORM_S8_UINT`：24 位归一化深度 + 8 位模板

### 子通道

子通道是渲染通道中的一个渲染步骤，描述本步骤引用哪些附件作为渲染目标

#### 附件引用

```cpp
VkAttachmentReference colorAttachmentRef = {};
colorAttachmentRef.attachment = 0;
colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

VkAttachmentReference depthAttachmentRef{};
depthAttachmentRef.attachment = 1;
depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
```

- `attachment`：指向 `pAttachments` 数组中的索引，0 为颜色附件，1 为深度附件
- `layout`：子通道执行期间该附件应处于的布局。Vulkan 会在子通道开始时自动将附件转换到此布局

#### 子通道描述

```cpp
VkSubpassDescription subpass = {};
subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
subpass.colorAttachmentCount = 1;
subpass.pColorAttachments = &colorAttachmentRef;
subpass.pDepthStencilAttachment = &depthAttachmentRef;
```

- `pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS`：这是图形子通道 (而非计算)
- `colorAttachmentCount = 1`：颜色附件数量
- `pColorAttachments`：颜色附件引用数组，片段着色器输出的颜色将写入此附件
- `pDepthStencilAttachment`：深度/模板附件引用，深度测试使用此附件

### 子通道依赖

```cpp
VkSubpassDependency dependency = {};
dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
dependency.srcAccessMask = 0;
dependency.srcStageMask =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
dependency.dstSubpass = 0;
dependency.dstStageMask =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
dependency.dstAccessMask =
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
```

子通道依赖描述了渲染通道隐含的同步关系：

- `srcSubpass = VK_SUBPASS_EXTERNAL`：依赖的源是渲染通道之前的外部操作
- `srcStageMask`：外部操作可能涉及颜色输出和早期片段测试阶段
- `dstSubpass = 0`：目标是本渲染通道的第一个 (唯一)子通道
- `dstAccessMask`：本子通道需要颜色写入和深度/模板写入权限

这确保了：在交换链图像从 `UNDEFINED` / `PRESENT_SRC` 布局转换到 `COLOR_ATTACHMENT_OPTIMAL` 布局之前，不会开始颜色输出

#### 为什么需要子通道依赖？

考虑这个场景：上一帧的呈现操作可能还在读取交换链图像 (`PRESENT_SRC_KHR` 布局)，而本帧的渲染通道要往同一张图像写入颜色

如果没有依赖声明，GPU 可能会同时执行这两个操作，导致数据竞争

子通道依赖告诉 GPU：必须等外部操作 (上一帧的呈现)完成后，才能开始本子通道的颜色写入和深度写入

#### srcStageMask 和 dstStageMask 的含义

```
srcStageMask (外部操作可能触及的阶段):          dstStageMask (本子通道需要用到的阶段):
  ┌──────────────────────────────┐              ┌──────────────────────────────┐
  │ COLOR_ATTACHMENT_OUTPUT_BIT  │              │ COLOR_ATTACHMENT_OUTPUT_BIT  │
  │ EARLY_FRAGMENT_TESTS_BIT    │              │ EARLY_FRAGMENT_TESTS_BIT    │
  └──────────────────────────────┘              └──────────────────────────────┘
         ↑                                              ↑
   上一帧可能还在输出颜色                        本帧要写入颜色和深度
   或执行早期片段测试
```

- `COLOR_ATTACHMENT_OUTPUT_BIT`：颜色输出阶段 (片段着色器输出 → 颜色附件)
- `EARLY_FRAGMENT_TESTS_BIT`：早期片段测试阶段 (深度/模板测试在片段着色器之前执行)

### 渲染通道创建

将附件、子通道、依赖汇总到一起，创建渲染通道对象

```cpp
std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
VkRenderPassCreateInfo renderPassInfo = {};
renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
renderPassInfo.pAttachments = attachments.data();
renderPassInfo.subpassCount = 1;
renderPassInfo.pSubpasses = &subpass;
renderPassInfo.dependencyCount = 1;
renderPassInfo.pDependencies = &dependency;

vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass);
```

- `attachmentCount = 2`：2 个附件 (颜色 + 深度)
- `subpassCount = 1`：1 个子通道
- `dependencyCount = 1`：1 个子通道依赖

#### 附件索引汇总

整个渲染通道中，附件通过索引关联：

```
pAttachments 数组:
  [0] colorAttachment  ←── colorAttachmentRef.attachment = 0
  [1] depthAttachment  ←── depthAttachmentRef.attachment = 1

帧缓冲区附件:
  [0] swapChainImageViews[i]  ←── 对应颜色附件
  [1] depthImageViews[i]      ←── 对应深度附件
```

渲染通道的附件索引、子通道的附件引用、帧缓冲区的附件数组——三者的索引必须一一对应

## 深度资源 (createDepthResources)

为交换链中的每一帧图像创建对应的深度图像和深度图像视图

```cpp
VkImageCreateInfo imageInfo{};
imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
imageInfo.imageType = VK_IMAGE_TYPE_2D;
imageInfo.extent.width = swapChainExtent.width;
imageInfo.extent.height = swapChainExtent.height;
imageInfo.extent.depth = 1;
imageInfo.mipLevels = 1;
imageInfo.arrayLayers = 1;
imageInfo.format = depthFormat;
imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
```

- `tiling = VK_IMAGE_TILING_OPTIMAL`：使用最优平铺方式，GPU 访问效率更高
- `usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT`：用作深度/模板附件
- 每个交换链图像都有对应的深度图像，所以深度图像的数量等于 `imageCount()`

深度图像视图的创建与交换链图像视图类似，区别在于 `aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT`，只访问深度通道

## 帧缓冲区 (createFramebuffers)

帧缓冲区将颜色附件和深度附件绑定在一起，作为渲染通道的渲染目标

```cpp
std::array<VkImageView, 2> attachments = {swapChainImageViews[i], depthImageViews[i]};

VkFramebufferCreateInfo framebufferInfo = {};
framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
framebufferInfo.renderPass = renderPass;
framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
framebufferInfo.pAttachments = attachments.data();
framebufferInfo.width = swapChainExtent.width;
framebufferInfo.height = swapChainExtent.height;
framebufferInfo.layers = 1;
```

- `renderPass`：帧缓冲区关联的渲染通道，附件数量和类型必须与渲染通道定义一致
- `pAttachments`：附件数组，索引 0 为颜色图像视图，索引 1 为深度图像视图——与渲染通道中的 `attachment` 字段对应
- `layers = 1`：层数，通常为 1

### 附件索引对应关系

```
渲染通道定义：                    帧缓冲区附件：
  colorAttachmentRef.attachment = 0    ←→    attachments[0] = swapChainImageViews[i]
  depthAttachmentRef.attachment = 1    ←→    attachments[1] = depthImageViews[i]
```

每个交换链图像都有独立的帧缓冲区，所以帧缓冲区数量也等于 `imageCount()`

## 同步对象 (createSyncObjects)

同步对象用于协调 CPU、GPU 和交换链之间的时序，避免同时访问同一资源导致冲突

### 同步原语

| 类型 | 作用 | 类比 |
| :--: | :--- | :--- |
| **信号量 (Semaphore)** | GPU-GPU 同步，通知某个操作已完成 | 红绿灯：一个 GPU 阶段等另一个 GPU 阶段 |
| **栅栏 (Fence)** | GPU-CPU 同步，让 CPU 知道 GPU 操作已完成 | 快递通知：GPU 干完后通知 CPU |

### 本示例使用的同步对象

```cpp
static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

std::vector<VkSemaphore> imageAvailableSemaphores;   // 信号量：交换链图像已就绪
std::vector<VkSemaphore> renderFinishedSemaphores;    // 信号量：渲染已完成
std::vector<VkFence> inFlightFences;                  // 栅栏：当前帧的 GPU 工作是否完成
std::vector<VkFence> imagesInFlight;                  // 栅栏：每个图像是否正在被使用
```

```cpp
VkFenceCreateInfo fenceInfo = {};
fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
```

- `VK_FENCE_CREATE_SIGNALED_BIT`：栅栏初始状态为已信号，表示第一帧无需等待

### 为什么需要 MAX_FRAMES_IN_FLIGHT？

`MAX_FRAMES_IN_FLIGHT = 2` 表示最多允许 2 帧同时在 GPU 上执行 (CPU 领先 GPU 最多 2 帧)

- 如果不限制，CPU 可能无限超前提交命令，导致命令队列无限增长、内存暴涨
- 限制为 2 帧，CPU 在提交第 3 帧前必须等待第 1 帧完成，保证资源可复用

同步对象的数量为 `MAX_FRAMES_IN_FLIGHT`，而非 `imageCount()`，因为它们按帧索引复用

#### 为什么 Semaphore 不能等 CPU？

这是因为信号量与栅栏的底层设计不同：
- 信号量 (Semaphore)：是 GPU 硬件层面执行的指令调度锁 (显卡内部的信号灯)，**CPU 无法直接查询它的状态**
- 栅栏 (Fence)：是 CPU 和 GPU 之间通信的桥梁 (就像快递通知)，CPU 可以调用 `vkWaitForFences` 把自己挂起等待

所以 GPU 等 GPU 用信号量，CPU 等 GPU 用栅栏。这就是为什么 `vkAcquireNextImageKHR` 用信号量接图像，而 `vkQueueSubmit` 用栅栏通知 CPU

## 获取下一帧图像 (acquireNextImage)

从交换链中获取下一个可呈现的图像索引

```cpp
vkWaitForFences(
    device.device(), 1, &inFlightFences[currentFrame],
    VK_TRUE, std::numeric_limits<uint64_t>::max());

VkResult result = vkAcquireNextImageKHR(
    device.device(), swapChain,
    std::numeric_limits<uint64_t>::max(),
    imageAvailableSemaphores[currentFrame],
    VK_NULL_HANDLE,
    imageIndex);
```

1. `vkWaitForFences`：等待当前帧的栅栏，确保上一轮使用同一同步对象的帧已完成
2. `vkAcquireNextImageKHR`：从交换链获取下一个可用图像的索引
    - 超时时间设为 `UINT64_MAX` (无限等待)
    - `imageAvailableSemaphores[currentFrame]`：当图像就绪时，此信号量会被信号
    - 传 `VK_NULL_HANDLE` 作为栅栏参数，因为我们用信号量来做 GPU-GPU 同步

## 提交命令缓冲区与呈现 (submitCommandBuffers)

将渲染命令提交到图形队列，然后将渲染结果呈现到屏幕

### 1. 等待图像就绪

```cpp
if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
}
imagesInFlight[*imageIndex] = inFlightFences[currentFrame];
```

如果该图像正在被上一帧使用，等待其完成。然后将当前帧的栅栏记录到 `imagesInFlight` 中

### 2. 提交渲染命令

```cpp
VkSubmitInfo submitInfo = {};
submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
submitInfo.waitSemaphoreCount = 1;
submitInfo.pWaitSemaphores = waitSemaphores;
submitInfo.pWaitDstStageMask = waitStages;

submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = buffers;

VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
submitInfo.signalSemaphoreCount = 1;
submitInfo.pSignalSemaphores = signalSemaphores;
```

提交的同步逻辑：

- **等待**：`imageAvailableSemaphores` — 等到交换链图像就绪后才开始执行命令
- `waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`：只需等到颜色输出阶段即可，不必等到图像完全就绪
- **信号**：`renderFinishedSemaphores` — 命令执行完成后发出信号，通知呈现操作可以开始

```cpp
vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);
```

- `vkResetFences`：将栅栏重置为未信号状态 (必须在提交前重置)
- `vkQueueSubmit`：提交到图形队列，并关联栅栏。GPU 完成后栅栏会被信号

### 3. 呈现

```cpp
VkPresentInfoKHR presentInfo = {};
presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

presentInfo.waitSemaphoreCount = 1;
presentInfo.pWaitSemaphores = signalSemaphores;

VkSwapchainKHR swapChains[] = {swapChain};
presentInfo.swapchainCount = 1;
presentInfo.pSwapchains = swapChains;

presentInfo.pImageIndices = imageIndex;

auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);
```

- `pWaitSemaphores = signalSemaphores`：等待渲染完成信号量，确保渲染结束后才呈现
- `vkQueuePresentKHR`：将图像提交到呈现队列，显示到屏幕

### 4. 推进帧计数

```cpp
currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
```

`currentFrame` 在 `[0, MAX_FRAMES_IN_FLIGHT)` 范围内循环，复用同步对象

### 完整同步流程

```
帧 N:
  CPU: acquireNextImage → 等待 inFlightFences[N%2]
  GPU: 等待 imageAvailableSemaphore[N%2] → 执行命令 → 信号 renderFinishedSemaphore[N%2]
  GPU: 等待 renderFinishedSemaphore[N%2] → 呈现到屏幕
  GPU: 完成后信号 inFlightFences[N%2]
  CPU: currentFrame = (N+1) % 2
```

```
时间线：

CPU:  ── acquire ── submit ── acquire ── submit ── ...
GPU:  ──── 等待图像就绪 ── 渲染帧0 ── 呈现帧0 ── 渲染帧1 ── 呈现帧1 ── ...

信号量:  imageAvailable[0]  ──────→  渲染开始
         renderFinished[0]  ──────→  呈现开始

栅栏:    inFlightFences[0] ──────→  CPU 可以复用该帧的同步对象
```

## 管线布局 (createPipelineLayout)

管线布局 (Pipeline Layout) 描述了管线可以访问的全局资源：描述符集和推送常量

```cpp
VkPipelineLayoutCreateInfo layoutCreateInfo = {};
layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
layoutCreateInfo.setLayoutCount = 0;
layoutCreateInfo.pSetLayouts = nullptr;
layoutCreateInfo.pushConstantRangeCount = 0;
layoutCreateInfo.pPushConstantRanges = nullptr;

vkCreatePipelineLayout(device.device(), &layoutCreateInfo, nullptr, &pipelineLayout);
```

- `setLayoutCount = 0` / `pSetLayouts = nullptr`：不使用任何描述符集布局。当前着色器不需要 Uniform Buffer、纹理采样器等资源
- `pushConstantRangeCount = 0` / `pPushConstantRanges = nullptr`：不使用推送常量。推送常量是一种快速更新少量着色器数据的方式（后续章节会介绍）

### 为什么需要管线布局？

即使当前不使用任何描述符集和推送常量，管线布局仍然是必需的——它是图形管线创建的必要参数

管线布局定义了管线的"资源接口"：

```
VkPipelineLayout
  ├── 描述符集布局 (Descriptor Set Layouts)
  │     描述着色器可以访问的 Uniform Buffer、纹理、采样器等资源的绑定方式
  │
  └── 推送常量范围 (Push Constant Ranges)
        描述着色器可以快速访问的小块常量数据的布局
```

- **当前阶段**：着色器硬编码顶点数据，不需要外部资源，所以全部为 0 / nullptr
- **后续阶段**：引入 Uniform Buffer（MVP 矩阵）、纹理采样器后，需要在此指定描述符集布局

## 图形管线创建 (createPipeline)

利用交换链的信息和管线布局，创建图形管线

```cpp
auto pipelineConfig = MyPipeline::defaultConfigInfo(swapChain.width(), swapChain.height());
pipelineConfig.renderPass = swapChain.getRenderPass();
pipelineConfig.pipelineLayout = pipelineLayout;

pipeline = std::make_unique<MyPipeline>(
    device,
    pipelineConfig,
    "simple_shader.vert.spv",
    "simple_shader.frag.spv");
```

### 步骤分解

1. **获取默认配置**：`MyPipeline::defaultConfigInfo(width, height)` 生成管线固定功能的默认配置（视口、光栅化、颜色混合等），传入交换链的宽高以设置视口和裁剪矩形

2. **设置渲染通道**：`pipelineConfig.renderPass = swapChain.getRenderPass()`
   - 管线必须知道在哪个渲染通道中执行，因为渲染通道定义了附件的格式和布局
   - 管线的子通道索引默认为 0（与渲染通道中唯一的子通道对应）

3. **设置管线布局**：`pipelineConfig.pipelineLayout = pipelineLayout`
   - 管线需要知道可以访问哪些描述符集和推送常量

4. **创建管线**：传入设备、配置、顶点着色器和片段着色器的 SPIR-V 文件路径

### 交换链与管线的关联

交换链为管线创建提供了两个关键信息：

```
VulkanSwapChain
  ├── width() / height()  ──→  视口和裁剪矩形的尺寸
  ├── getRenderPass()     ──→  管线执行所在的渲染通道
  └── getSwapChainImageFormat() ──→  颜色附件的格式 (管线颜色混合配置需匹配)
```

- **视口尺寸**：管线的视口和裁剪矩形必须与交换链图像尺寸一致，否则渲染结果会被裁剪或拉伸
- **渲染通道**：管线的渲染通道必须与帧缓冲区的渲染通道一致，附件格式必须匹配
- **图像格式**：管线的颜色混合配置必须与交换链的颜色格式兼容
