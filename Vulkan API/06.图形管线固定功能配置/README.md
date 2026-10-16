# 图形管线固定功能配置

本示例在"管线输出"的基础上，为 `PipelineConfigInfo` 填充 Vulkan 图形管线的全部固定功能阶段配置，并通过 `defaultConfigInfo` 提供合理的默认值。下面按 `defaultConfigInfo` 中的初始化顺序依次介绍每个成员。

## 输入装配（Input Assembler）

输入装配器会根据管线指定的图元拓扑，从顶点缓冲和可选的索引缓冲中读取数据，并组合成对应的图元 (例如三角形)

如果拓扑被指定为三角形列表，它就会把数据组合为三角形的顶点数据

```cpp
VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
```

```cpp
configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
```

- `topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST`：每 3 个顶点构成一个三角形，最常用的图元拓扑
- `primitiveRestartEnable = VK_FALSE`：不启用图元重启（仅在使用索引缓冲区绘制带状/扇形图元时才有意义）

### 为什么需要图元拓扑？

假设现在有三个坐标 `(0, 1)`、`(1, 1)` 和 `(2, 1)`

那这三个坐标可能有以下组合

1. 三个单独的点
2. 连成两条线段 (如 `(0, 1)` 到 `(1, 1)`，`(1, 1)` 到 `(2, 1)`)
3. 连成一个三角形

这就需要图元拓扑来指定

有关所有图元拓扑参数的详细信息，请参考 [Vulkan 官方文档](https://docs.vulkan.org/refpages/latest/refpages/source/VkPrimitiveTopology.html)

## 视口 (Viewport)

视口是 NDC 坐标到帧缓冲像素坐标的映射范围，是渲染管线的一个固定功能阶段

NDC (Normalized Device Coordinate) 是归一化设备坐标，

NDC 坐标的范围是:

- x: `[-1, 1]`
- y: `[-1, 1]`
- z: `[0, 1]` (Vulkan 的 NDC z 范围，OpenGL 是 `[-1, 1]`)

假设窗口大小 800×600

那么映射是这样的

| NDC 坐标 | 映射后帧缓冲坐标 |
| :-----: | :------------: |
| `(-1, -1)` | `(0, 0)` — 左上角 |
| `(0, 0)` | `(400, 300)` — 中心 |
| `(1, 1)` | `(800, 600)` — 右下角 |

```cpp
VkViewport viewport;
```

```cpp
configInfo.viewport.x = 0.0f;
configInfo.viewport.y = 0.0f;
configInfo.viewport.width = static_cast<float>(width);
configInfo.viewport.height = static_cast<float>(height);
configInfo.viewport.minDepth = 0.0f;
configInfo.viewport.maxDepth = 1.0f;
```

视口定义 NDC 坐标到帧缓冲像素坐标的映射范围：

- `x` / `y`：视口左上角在帧缓冲中的像素偏移，默认 `(0, 0)` 即从左上角开始
- `width` / `height`：视口的像素尺寸，与窗口宽高一致
- `minDepth` / `maxDepth`：深度值的映射范围，默认 `[0, 1]`，对应 NDC 的 z 范围

## 裁剪矩形 (Scissor)

裁剪矩形是一个矩形区域，用于限制渲染的片段范围

它定义一个矩形区域，视口变换后的片段只有落在该区域内才会被保留

```cpp
VkRect2D scissor;
```

```cpp
configInfo.scissor.offset = {0, 0};
configInfo.scissor.extent = {width, height};
```

- `offset`：矩形左上角的像素偏移
    - 例如输入 `{50, 50}`，则矩形左上角在 `(50, 50)` 处，右下角超出 `extent` 的部分会被裁剪
- `extent`：矩形的像素尺寸
    - 例如输入 `height * 0.5`，则只会显示图形的上半部分

### 示例

假设窗口大小 800×600

| offset | extent | 效果 |
| :----: | :----: | :--: |
| `{0, 0}` | `{800, 600}` | 裁剪矩形覆盖整个帧缓冲，所有片段都保留 (默认) |
| `{100, 100}` | `{800, 600}` | 矩形左上角移到 `(100,100)`，右下角超出帧缓冲范围<br>超出部分被 Vulkan 自动钳制到帧缓冲边界，最终裁剪区域为 `(100,100)`→`(800,600)` |
| `{100, 100}` | `{400, 300}` | 裁剪区域为 `(100,100)`→`(500,400)`<br>只有落在这个矩形内的片段会通过，矩形外的片段被丢弃 |

默认裁剪矩形覆盖整个视口，即不做额外裁剪

### 与 ViewPort 的区别

- Viewport：做坐标变换，决定 NDC 空间如何映射到像素空间 (缩放 + 偏移)
- Scissor：做矩形裁剪，在像素空间中定义一个矩形，矩形外的片段直接丢弃

## 视口状态 (Viewport State)

视口状态的作用是 把 Viewport 和 Scissor 组装成一个管线状态，告诉管线"使用哪些视口和裁剪矩形"

```cpp
VkPipelineViewportStateCreateInfo viewportInfo;
```

```cpp
configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
configInfo.viewportInfo.viewportCount = 1;
configInfo.viewportInfo.pViewports = &configInfo.viewport;
configInfo.viewportInfo.scissorCount = 1;
configInfo.viewportInfo.pScissors = &configInfo.scissor;
```

- `viewportCount` / `scissorCount`：视口和裁剪矩形的数量，这里各 1 个
- `pViewports` / `pScissors`：指向视口和裁剪矩形对象的指针

### 为什么需要这一层？

Vulkan 管线创建时，每个固定功能阶段都需要一个对应的 `CreateInfo` 结构体

视口和裁剪属于同一个管线阶段，所以 Vulkan 用一个 `VkPipelineViewportStateCreateInfo` 统一管理：

```cpp
typedef struct VkPipelineViewportStateCreateInfo {
    VkStructureType sType;
    const void*     pNext;
    VkPipelineViewportStateCreateFlags flags;
    uint32_t        viewportCount;    // 视口数量
    const VkViewport* pViewports;     // 指向视口数组的指针
    uint32_t        scissorCount;     // 裁剪矩形数量
    const VkRect2D*  pScissors;       // 指向裁剪矩形数组的指针
} VkPipelineViewportStateCreateInfo;
```

#### 常见用法——多视口/多裁剪

`viewportCount` 和 `scissorCount` 可以大于 1，这就是 多视口渲染 (Multi-Viewport)

GPU 可以在一次绘制调用中同时渲染到多个不同的视口区域。几何着色器可以通过 `gl_ViewportIndex` 选择每个图元输出到哪个视口

### 为什么视口和裁剪矩形要作为独立成员存储在 PipelineConfigInfo 中？

因为 `pViewports` 和 `pScissors` 是指针，它们必须指向在管线创建时仍然有效的 `VkViewport` 和 `VkRect2D` 对象。如果将视口和裁剪矩形声明为函数局部变量，函数返回后指针就会悬空。将它们作为结构体成员，保证生命周期与 `PipelineConfigInfo` 一致

## 光栅化 (Rasterization)

光栅化器将顶点着色器输出的三角形转换为屏幕上的像素片段（fragment）。在此之前，GPU 会自动完成裁剪、透视除法等固定步骤

```cpp
VkPipelineRasterizationStateCreateInfo rasterizationInfo;
```

```cpp
configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
configInfo.rasterizationInfo.lineWidth = 1.0f;
configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;
```

- `depthClampEnable = VK_FALSE`：不钳制超出近远平面的片段，超出范围的片段直接丢弃
- `rasterizerDiscardEnable = VK_FALSE`：不禁用光栅化，几何体正常输出片段
- `polygonMode = VK_POLYGON_MODE_FILL`：多边形填充模式（还可选 `LINE` 线框、`POINT` 点）
- `cullMode = VK_CULL_MODE_NONE`：不进行背面剔除
- `frontFace = VK_FRONT_FACE_CLOCKWISE`：顺时针绕序为正面
- `depthBiasEnable = VK_FALSE`：不启用深度偏移（阴影贴图等场景需要开启）

## 多重采样 (Multisampling)

多重采样 (Multisampling) 是一种抗锯齿技术，全称 MSAA (Multisample Anti-Aliasing)

### 锯齿是怎么产生的

光栅化阶段会将连续的几何图元离散化为像素网格。在三角形边缘处，一个像素要么完全在三角形内（着色），要么完全在外（不着色），产生锯齿状的阶梯：

```
无 MSAA：          有 MSAA：
███░░░░░          ██▓░░░░░
████░░░░          ███▓░░░░
█████░░░          ████▓░░░
██████░░          █████▓░░
```

- █ = 完全覆盖
- ░ = 完全未覆盖
- ▓ = 部分覆盖 (MSAA 的关键)

```cpp
VkPipelineMultisampleStateCreateInfo multisampleInfo;
```

```cpp
configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
configInfo.multisampleInfo.minSampleShading = 1.0f;
configInfo.multisampleInfo.pSampleMask = nullptr;
configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;
```

- `rasterizationSamples = VK_SAMPLE_COUNT_1_BIT`：每个像素仅采样 1 次，即不启用多重采样抗锯齿（MSAA）
- `sampleShadingEnable = VK_FALSE`：不启用逐采样着色
- 其余字段均为多重采样高级选项，当前保持默认关闭

## 颜色混合 (Color Blending)

颜色混合分两层配置：

- 逐附件 (attachment): 单个颜色附件的混合方式
- 全局 (state): 跨附件的全局混合行为

两者的关系如下

```
VkPipelineColorBlendStateCreateInfo (全局)
  ├── attachmentCount = 2
  ├── pAttachments ──→ [ 附件0: ColorBlendAttachmentState,
  │                        附件1: ColorBlendAttachmentState ]
  ├── logicOpEnable = VK_FALSE
  └── blendConstants = {0, 0, 0, 0}

VkPipelineColorBlendAttachmentState (附件0)  ← 逐附件独立配置
VkPipelineColorBlendAttachmentState (附件1)  ← 逐附件独立配置
```

总的来说逐附件定义每个颜色附件各自的混合公式，全局配置将它们汇总并管理所有附件共享的参数 (逻辑运算开关、混合常量)

### 逐附件配置

逐附件配置管的是单个颜色附件的混合方式

一个渲染通道可以有多个颜色附件 (多个渲染目标)，每个附件可以有不同的混合配置

```cpp
VkPipelineColorBlendAttachmentState colorBlendAttachment;
```

```cpp
configInfo.colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT;
configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
```

- `blendEnable = VK_FALSE`：不启用混合，片段颜色直接覆盖帧缓冲
- `colorWriteMask`：写入 RGBA 全部四个通道
- 混合公式为 `src * srcFactor OP dst * dstFactor`，当 `blendEnable = VK_FALSE` 时混合因子不生效，但必须设置合法值

### 全局配置

全局配置管的是跨附件的全局混合行为。它不定义混合公式本身，而是：

- 关联所有逐附件配置：`pAttachments` 指向一个 `VkPipelineColorBlendAttachmentState` 数组，attachmentCount` 指定数量
- 逻辑运算：`logicOpEnable` 开启后，对所有附件的颜色值做位运算（AND、OR、XOR 等），与上面的混合公式互斥
- 混合常量：`blendConstants[4]`，当混合因子使用 `VK_BLEND_FACTOR_CONSTANT_COLOR` 等时，从这里取常量值

```cpp
VkPipelineColorBlendStateCreateInfo colorBlendInfo;
```

```cpp
configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
configInfo.colorBlendInfo.attachmentCount = 1;
configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
configInfo.colorBlendInfo.blendConstants[3] = 0.0f;
```

- `logicOpEnable = VK_FALSE`：不启用逻辑运算混合
- `attachmentCount = 1`：仅一个颜色附件
- `pAttachments` 指向逐附件配置
    - 与视口类似，`pAttachments` 也是指针，**必须保证 `colorBlendAttachment` 的生命周期长于管线创建时间**

## 深度/模板测试 (Depth / Stencil)

### 深度测试

解决的核心问题：当多个图元投影到同一个像素时，谁应该在前面？

```
场景侧视图：

  相机 ──→  △ 近处的三角形
            □ 远处的正方形

投影到帧缓冲后，两个图元覆盖同一个像素区域
```

没有深度测试时，后绘制的总是覆盖先绘制的（画家算法），远处物体可能遮挡近处物体——这是错误的

深度测试的机制：

1. 每个像素在深度缓冲区中存储一个深度值 (0.0~1.0，0 最近，1 最远)
2. 新片段到来时，将其深度值与深度缓冲中已有值比较
3. 比较规则由 `depthCompareOp` 决定
4. 通过测试 → 写入颜色和深度；未通过 → 丢弃片段

```
深度缓冲初始值 = 1.0（最远）

片段A 深度 0.3 → 0.3 < 1.0 ✓ 通过，写入颜色，深度缓冲更新为 0.3
片段B 深度 0.7 → 0.7 < 0.3 ✗ 未通过，丢弃
片段C 深度 0.1 → 0.1 < 0.3 ✓ 通过，写入颜色，深度缓冲更新为 0.1
```

### 模板测试

解决的核心问题：根据像素的模板标记，决定是否绘制该像素。

模板测试使用一个额外的模板缓冲区，每个像素存储一个整数模板值。绘制时可以：

1. 读取当前像素的模板值
2. 与参考值做比较 (等于、不等于、大于等)
    - 比较未通过 → 丢弃片段
    - 比较通过 → 根据操作规则更新模板值 (保持、置零、递增、递减等)

可以理解为一个更高级的裁剪矩形 (Scissor)，但模板测试可以更灵活地裁出**任何复杂的形状**

| 特征 | 裁剪矩形 (Scissor) | 模板测试 (Stencil Test) |
| :--: | :--------------: | :--------------------: |
| 裁剪形状 | 只能是矩形 (Rectangle)<br>只能定义左上角偏移 (offset) 和宽高 (extent) | 可以是任意极其复杂的、不规则的形状<br>圆、心形、非凸多边形、文字轮廓、甚至 3D 模型的投影 |
| 控制粒度 | 粗暴的范围划分<br>处于矩形框外的片段直接一刀切物理丢弃，无法做更细的控制 | 像素级别的数字开关<br>每个像素根据“隐形黑板”上的值进行独立的位运算 (AND/OR) 和条件判定 |
| 数据动态性 | 静态/单向的<br>CPU 在录制命令时写死矩形大小，渲染过程中无法根据画的东西动态改变裁剪范围 | 动态/双向互动的<br> GPU 可以边画边改裁剪区域 (比如前面画的物体把模板值改成 1，后面画的物体顺着 1 的标记去裁剪) |

#### 典型应用：镜面/门户渲染

```
第一步：画镜子形状，向模板缓冲写入 1
┌─────────────┐
│             │
│   ┌─────┐   │   模板缓冲：镜子区域 = 1，其余 = 0
│   │  1  │   │
│   └─────┘   │
│             │
└─────────────┘

第二步：画镜像场景，只在模板值 = 1 的地方绘制
┌─────────────┐
│             │
│   ┌─────┐   │   镜像场景只出现在镜子区域内
│   │镜像 │   │
│   └─────┘   │
│             │
└─────────────┘
```

```cpp
VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
```

```cpp
configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
configInfo.depthStencilInfo.minDepthBounds = 0.0f;
configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
configInfo.depthStencilInfo.front = {};
configInfo.depthStencilInfo.back = {};
```

- `depthTestEnable = VK_TRUE`：启用深度测试，距离相机更近的片段才会写入帧缓冲
- `depthWriteEnable = VK_TRUE`：通过深度测试的片段会更新深度缓冲
- `depthCompareOp = VK_COMPARE_OP_LESS`：深度值更小的片段通过测试（更近）
- `depthBoundsTestEnable = VK_FALSE`：不启用深度范围测试
- `stencilTestEnable = VK_FALSE`：不启用模板测试

## 管线布局与渲染通道

```cpp
VkPipelineLayout pipelineLayout = nullptr;
VkRenderPass renderPass = nullptr;
uint32_t subpass = 0;
```

这三个成员在结构体定义中直接给出默认值，`defaultConfigInfo` 中不再显式赋值，由调用方在使用前设置：

- `pipelineLayout`：管线布局，描述着色器使用的描述符集和推送常量
- `renderPass`：渲染通道，管线将在此渲染通道内执行
- `subpass`：子通道索引，指定管线对应的子通道

**注意**：此时 `pipelineLayout` 默认为 `nullptr`，这在没有纹理和顶点缓冲区时是合法的，但一旦你引入 Uniform Buffer 或纹理 (也就是后续的 Descriptor Se)），这里必须传入合法的 `VkPipelineLayout` 对象，否则管线创建会失败

## 着色器阶段 (Shader Stages)

着色器阶段的作用是告诉管线"这个阶段用哪个着色器模块、从哪个入口函数开始执行"

```cpp
VkPipelineShaderStageCreateInfo shaderStages[2];

shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
shaderStages[0].module = vertShaderModule;
shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
shaderStages[1].module = fragShaderModule;

for (uint8_t i = 0; i < 2; i++)
{
    shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[i].pName = "main";
    shaderStages[i].pNext = nullptr;
    shaderStages[i].flags = 0;
    shaderStages[i].pSpecializationInfo = nullptr;
}
```

每个着色器阶段由 `VkPipelineShaderStageCreateInfo` 描述：

- `stage`：着色器阶段类型，顶点着色器为 `VK_SHADER_STAGE_VERTEX_BIT`，片段着色器为 `VK_SHADER_STAGE_FRAGMENT_BIT`
- `module`：之前由 `createShaderModule` 创建的 `VkShaderModule`
- `pName = "main"`：着色器入口函数名，SPIR-V 中默认为 `main`
- `pSpecializationInfo = nullptr`：不使用着色器常量特化

两个阶段先设置各自独有的 `stage` 和 `module`，再通过循环填充公共字段，避免重复代码。

## 顶点输入状态（Vertex Input State）

```cpp
VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vertexInputInfo.vertexBindingDescriptionCount = 0;
vertexInputInfo.vertexAttributeDescriptionCount = 0;
vertexInputInfo.pVertexBindingDescriptions = nullptr;
vertexInputInfo.pVertexAttributeDescriptions = nullptr;
```

顶点输入状态描述顶点数据的格式和布局：

- `vertexBindingDescriptionCount = 0`：无顶点绑定描述，即不从顶点缓冲区读取数据
- `vertexAttributeDescriptionCount = 0`：无顶点属性描述

当前着色器直接在顶点着色器中硬编码顶点数据，因此顶点输入为空。后续引入顶点缓冲区后会填充这些字段。

## 图形管线创建 (Graphics Pipeline Creation)

这一步将我们之前创建的着色器阶段、顶点输入状态、输入装配器、视口与裁剪、光栅化器、颜色混合器、深度/模板测试、管线布局、渲染通道与子通道索引等配置汇总到一起，创建成一个图形管线

```cpp
VkGraphicsPipelineCreateInfo pipelineInfo{};
pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
pipelineInfo.stageCount = 2;
pipelineInfo.pStages = shaderStages;
pipelineInfo.pVertexInputState = &vertexInputInfo;
pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
pipelineInfo.pViewportState = &configInfo.viewportInfo;
pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
pipelineInfo.pDynamicState = nullptr;

pipelineInfo.layout = configInfo.pipelineLayout;
pipelineInfo.renderPass = configInfo.renderPass;
pipelineInfo.subpass = configInfo.subpass;

pipelineInfo.basePipelineIndex = -1;
pipelineInfo.basePipelineHandle = nullptr;
```

`VkGraphicsPipelineCreateInfo` 是管线创建的总装结构体，将前面所有配置汇总到一起：

- `stageCount` / `pStages`：着色器阶段数组，2 个（顶点 + 片段）
- `pVertexInputState`：顶点输入状态
- `pInputAssemblyState`：输入装配
- `pViewportState`：视口与裁剪
- `pRasterizationState`：光栅化
- `pMultisampleState`：多重采样
- `pColorBlendState`：颜色混合
- `pDepthStencilState`：深度/模板测试
- `pDynamicState = nullptr`：不使用动态状态
    - 动态状态 (Dynamic States) 是指在管线执行过程中可以动态改变的状态，例如动态修改视口的大小、位置、裁剪范围等
- `layout` / `renderPass` / `subpass`：管线布局、渲染通道与子通道索引
- `basePipelineHandle` / `basePipelineIndex`：管线派生相关，`-1` 和 `nullptr` 表示不从已有管线派生
    - 可以利用“管线派生（Pipeline Derivatives）”让多个相似管线共享基础状态，大幅加快管线创建速度

最后调用 `vkCreateGraphicsPipelines` 创建管线：

```cpp
if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
{
    throw std::runtime_error("Failed to create graphics pipeline");
}
```

- 第一个参数：逻辑设备
- 第二个参数：管线缓存（`VK_NULL_HANDLE` 表示不使用）
- 第三个参数：创建信息数量
- 第四个参数：指向 `VkGraphicsPipelineCreateInfo` 数组的指针
- 第五个参数：自定义分配器
- 第六个参数：输出创建好的 `VkPipeline` 句柄

## 析构函数

```cpp
MyPipeline::~MyPipeline()
{
    vkDestroyPipeline(device.device(), pipeline, nullptr);
    vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
    vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
}
```

析构时按创建的逆序销毁 Vulkan 对象：

1. 销毁顶点着色器模块
2. 销毁片段着色器模块
3. 销毁管线对象

着色器模块在管线创建完成后不再被引用，但它们是独立的 Vulkan 对象，必须显式销毁。管线本身也必须显式销毁
