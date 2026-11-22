# 广告牌

广告牌 (Billboard) 是 Vulkan 中一种让平面几何体始终朝向摄像机的渲染技术，常用于实现树木、粒子、光晕等无论视角如何变化都正对观察者的效果

本质上，广告牌是嵌入在 3D 空间中的一块 2D 平面几何体，只是额外具备一个属性——始终朝向摄像机。效果就如同画面中总有一张脸在注视着你

常见应用场景如下：

- 显示在敌人头顶的血量条
- 表现物理上难以模拟的透明物体，例如云、烟雾、粒子等

## 广告牌分类

广告牌共分为三类：

- 视角对齐广告牌 (View Aligned Billboard / Screen Aligned Billboard)
  - 朝向仅由摄像机的旋转决定，与广告牌自身的位置无关
- 面向视口广告牌 (Viewport Oriented Billboard / World Oriented Billboard)
  - 朝向同时由摄像机旋转**和**广告牌位置决定
- 轴向广告牌 (Axial Billboard)
  - 朝向只绕某一固定轴旋转，通常用于呈现树木等植被

详细信息请参考[这篇文章](https://www.flipcode.com/archives/Billboarding-Excerpt_From_iReal-Time_Renderingi_2E.shtml)

本示例采用视角对齐广告牌

## 视角对齐广告牌的几何构造

视角对齐广告牌的四角由「光源位置 + 半径 × 相机的右向量与上向量」算出，这两个向量取自视图矩阵：

```glsl
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

const float LIGHT_RADIUS = 0.1;

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
    vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

    vec3 positionWorld = ubo.lightPosition.xyz
        + LIGHT_RADIUS * fragOffset.x * cameraRightWorld
        + LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

    gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}
```

- **`cameraRightWorld` 是视图矩阵的第 0 列**：`ubo.view[0][0]`、`ubo.view[1][0]`、`ubo.view[2][0]` 依次取出该列的 x、y、z 分量
- **`cameraUpWorld` 是视图矩阵的第 1 列**：`ubo.view[i][1]` 三个分量组成相机的上方向
- **这两列就是世界空间中的相机轴**：视图矩阵左上 3×3 是相机旋转基的转置，列方向即相机的右、上向量在世界坐标下的表示
- **列顺序与 GLSL 的列主序一致**：`ubo.view[a][b]` 的 `a` 是列、`b` 是行，因此第 0 列对应 `cameraRightWorld`
- **朝向只依赖 `view`**：两个轴向量都从视图矩阵取出，与广告牌自身的世界位置无关，这正是视角对齐广告牌的定义
- **`positionWorld` 以 `lightPosition` 为圆心**：广告牌贴在世界空间中的光源坐标上，与光照计算里用的是同一个字段
- **`LIGHT_RADIUS` 是世界空间半径**：`0.1` 与模型缩放无关，广告牌在远处会显得更小
- **`fragOffset` 是单位方形的局部坐标**：六个顶点分别落在 `(±1, ±1)`，由光栅化插值到每个片段
- **六个顶点组成两个三角形**：前三个构成左下、左上、右下，后三个构成右下、左上、右上
- **`gl_Position` 只做 `projection * view` 两次乘法**：`positionWorld` 已经是世界坐标，模型矩阵在算出它时已经用掉
- **顶点数据来自常量数组而非顶点缓冲**：`OFFSETS[gl_VertexIndex]` 用内建索引直接取数

## 全局统一缓冲区里的投影与视图

`GlobalUbo` 用两个 `mat4` 分别存放投影矩阵与视图矩阵：

```cpp
struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
  glm::vec3 lightPosition{-1.f};
  alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity
};
```

- **`projection` 与 `view` 各占 64 字节**：实测偏移分别是 0 与 64
- **`ambientLightColor` 偏移 128、`lightPosition` 偏移 144、`lightColor` 偏移 160**：与 std140 的 16 字节对齐规则一致
- **`sizeof(GlobalUbo)` 为 176**：`lightPosition` 之后仍有 4 字节填充，176 本身是 16 的倍数
- **`view` 在着色器里有两个用途**：既参与 `gl_Position` 的变换，也提供广告牌所需的右、上向量
- **视图矩阵的旋转部分直接可用**：世界空间的相机轴就藏在它的前两列里，无需另建一份相机朝向字段

帧循环里两个矩阵分别写入：

```cpp
ubo.projection = camera.getProjectionMatrix();
ubo.view = camera.getViewMatrix();
```

- **`projection` 只取投影矩阵**：不含视图部分，两者相乘的工作交给着色器
- **`view` 取视图矩阵**：包含相机位置与旋转
- **两次乘法在着色器里完成**：`ubo.projection * ubo.view * positionWorld`
- **乘积与两个分别存放的矩阵来自同一台相机**：`getProjectionMatrix()` 与 `getViewMatrix()` 由同一帧的 `camera` 提供

场景着色器同样声明这两个字段：

```glsl
layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor; // w is intensity
    vec3 lightPosition;
    vec4 lightColor;
} ubo;
```

```glsl
gl_Position = ubo.projection * ubo.view * positionWorld;
```

- **块内五个字段与 C++ 结构逐字对应**：`projection`、`view`、`ambientLightColor`、`lightPosition`、`lightColor`
- **`gl_Position` 由两次矩阵乘法得到**：`positionWorld` 先经 `view` 进入相机空间，再经 `projection` 进入裁剪空间
- **`positionWorld` 单独保留**：光照计算仍需要世界坐标，因此先算出它再复用

## 管线配置里的顶点输入描述

`PipelineConfigInfo` 携带两条顶点输入描述向量：

```cpp
struct PipelineConfigInfo {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  ...
};
```

- **两个成员都是 `std::vector`**：放在结构体最前面，长度由调用方决定
- **`bindingDescriptions` 描述顶点缓冲的步长与绑定编号**：对应 `VkVertexInputBindingDescription`
- **`attributeDescriptions` 描述每个属性的位置、格式与偏移**：对应 `VkVertexInputAttributeDescription`
- **长度为零表示该管线不读顶点缓冲**：`vertexBindingDescriptionCount` 与 `vertexAttributeDescriptionCount` 都取 `size()`，零长度即无顶点输入
- **每条管线各自持有一份配置**：`SimpleRenderSystem` 与 `PointLightSystem` 分别构造自己的 `PipelineConfigInfo`

`MyPipeline::createPipeline` 直接读取这两个成员：

```cpp
auto& bindingDescriptions = configInfo.bindingDescriptions;
auto& attributeDescriptions = configInfo.attributeDescriptions;

VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
```

- **`auto&` 取引用**：不复制向量，直接指向 `configInfo` 里的那份
- **计数由 `size()` 算出**：两个 `Count` 字段始终与向量长度一致
- **指针取自 `data()`**：`configInfo` 在 `vkCreateGraphicsPipelines` 调用期间有效，向量内容不会悬空

`defaultConfigInfo` 把 `MyModel::Vertex` 的布局填进去：

```cpp
configInfo.bindingDescriptions = MyModel::Vertex::getBindingDescriptions();
configInfo.attributeDescriptions = MyModel::Vertex::getAttributeDescriptions();
```

- **填充发生在函数末尾**：其余管线状态都设置完毕之后
- **默认值就是模型顶点格式**：位置、颜色、法线、UV 四个属性的绑定与偏移
- **调用方可以整份改写或清空**：这两个向量在 `configInfo` 上直接可改
- **静态函数返回向量**：`getBindingDescriptions()` 与 `getAttributeDescriptions()` 的返回值按值赋给成员

## 点光源系统

`PointLightSystem` 负责把光源位置画成一个圆形广告牌：

```cpp
class PointLightSystem {
 public:
  PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  ~PointLightSystem();

  PointLightSystem(const PointLightSystem &) = delete;
  PointLightSystem &operator=(const PointLightSystem &) = delete;

  void render(FrameInfo& frameInfo);

 private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  VulkanDevice& device;
  std::unique_ptr<MyPipeline> pipeline;
  VkPipelineLayout pipelineLayout;
};
```

- **构造函数接收三个参数**：设备、渲染通道、全局描述符集布局
- **`render` 只接收 `FrameInfo&`**：命令缓冲与描述符集都从帧信息里取
- **`device` 是引用成员**：管线与布局的销毁都要用到它
- **`pipeline` 用 `unique_ptr` 持有**：`MyPipeline` 的析构函数负责销毁 Vulkan 管线对象
- **`pipelineLayout` 是裸句柄**：在析构函数里显式 `vkDestroyPipelineLayout`
- **拷贝构造与拷贝赋值都被 `delete`**：对象不可复制
- **构造顺序是先布局后管线**：`createPipelineLayout` 在前，`createPipeline` 在后

构造与析构：

```cpp
PointLightSystem::PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : device(device) {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() { vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr); }
```

- **初始化列表只绑定 `device`**：另外两个成员在函数体里创建
- **析构函数只销毁布局**：管线由 `unique_ptr` 的析构负责

### 管线布局

```cpp
std::vector<VkDescriptorSetLayout> setLayouts{globalSetLayout};

VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
pipelineLayoutInfo.pSetLayouts = setLayouts.data();
pipelineLayoutInfo.pushConstantRangeCount = 0;
pipelineLayoutInfo.pPushConstantRanges = nullptr;
```

- **`setLayouts` 只有一个元素**：全局描述符集布局，与场景管线共用同一份
- **`pushConstantRangeCount` 为 0**：`pPushConstantRanges` 为 `nullptr`，这条管线不接收推送常量
- **广告牌的顶点位置完全由 `gl_VertexIndex` 与 UBO 推出**：没有逐物体参数需要推送
- **`setLayoutCount` 取自 `size()`**：字段值就是向量长度

### 管线配置

```cpp
PipelineConfigInfo pipelineConfigInfo{};
MyPipeline::defaultConfigInfo(pipelineConfigInfo);
pipelineConfigInfo.renderPass = renderPass;
pipelineConfigInfo.pipelineLayout = pipelineLayout;
pipeline = std::make_unique<MyPipeline>(
    device,
    pipelineConfigInfo,
    "point_light.vert.spv",
    "point_light.frag.spv");
```

- **`defaultConfigInfo` 先填入一整套状态**：`inputAssemblyInfo`、`viewportStateInfo`、`rasterizationInfo`、`multisampleInfo`、`colorBlendAttachment`、`colorBlendInfo`、`depthStencilInfo`、`dynamicStateInfo`
- **`renderPass` 与 `pipelineLayout` 显式指定**：渲染通道来自构造参数，布局是本类的 `pipelineLayout`
- **着色器是 `point_light.vert.spv` 与 `point_light.frag.spv`**：与场景的 `simple_shader.*` 分开
- **其余状态沿用 `defaultConfigInfo` 的设置**：`cullMode` 为 `VK_CULL_MODE_NONE`、`depthTestEnable` 与 `depthWriteEnable` 为 `VK_TRUE`、`depthCompareOp` 为 `VK_COMPARE_OP_LESS`
- **`defaultConfigInfo` 同时带来四条顶点属性描述**：location 依次是 0 到 3，来自 `MyModel::Vertex`
- **`point_light.vert` 一条都没有消费**：没有 `layout(location = N) in` 声明，顶点位置由 `gl_VertexIndex` 与 UBO 推出

### 顶点输入描述清空所解决的问题

#### 问题

这份配置交给 `vkCreateGraphicsPipelines()` 会报出四条错误：

```
validation layer: vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState Vertex attribute at location 0 not consumed by vertex shader.
validation layer: vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState Vertex attribute at location 1 not consumed by vertex shader.
validation layer: vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState Vertex attribute at location 2 not consumed by vertex shader.
validation layer: vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState Vertex attribute at location 3 not consumed by vertex shader.
```

这是因为 `MyPipeline::defaultConfigInfo` 会把 `MyModel::Vertex` 的四个属性填进 `configInfo.attributeDescriptions`，location 依次是 0 到 3：

```cpp
attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
```

- **四条报错的 location 与四个属性一一对应**：`0` 是 `position`、`1` 是 `color`、`2` 是 `normal`、`3` 是 `uv`
- **报错发生在管线创建阶段**：校验层比对的是 `pVertexInputState` 与顶点着色器字节码，与运行时是否真的调用 `vkCmdBindVertexBuffers` 无关
- **规则是「声明了就必须被消费」**：`pVertexInputState` 里列出的每个 `VkVertexInputAttributeDescription`，其 `location` 都要在顶点着色器里有对应的 `in` 声明并被实际读取

#### 解决方案

在 `PointLightSystem::createPipeline` 里把两条向量清空：

```cpp
MyPipeline::defaultConfigInfo(pipelineConfigInfo);
pipelineConfigInfo.bindingDescriptions.clear();
pipelineConfigInfo.attributeDescriptions.clear();
```

- **`attributeDescriptions.clear()` 使 `vertexAttributeDescriptionCount` 变成 0**：`pVertexInputState` 里没有任何属性条目，没有可比对的对象，四条报错不再触发
- **`bindingDescriptions.clear()` 使 `vertexBindingDescriptionCount` 也变成 0**：绑定描述给的是顶点缓冲的步长与绑定编号，属性为空后保留它没有意义
- **清空之后这条管线不读任何顶点缓冲**：`PointLightSystem::render` 里没有 `vkCmdBindVertexBuffers` 调用，顶点数据全部来自 `OFFSETS` 常量数组与 UBO
- **两条向量属于 `PipelineConfigInfo`**：场景管线保留默认值，广告牌管线清空，两套顶点输入描述各自独立

### 绘制

```cpp
void PointLightSystem::render(FrameInfo& frameInfo)
{
  pipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
    frameInfo.commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout,
    0,
    1,
    &frameInfo.globalDescriptorSet,
    0,
    nullptr);

    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}
```

- **先绑定管线**：`pipeline->bind` 调用 `vkCmdBindPipeline`
- **描述符集绑定在 `set = 0`**：`firstSet` 为 0、`descriptorSetCount` 为 1、`dynamicOffsetCount` 为 0
- **没有 `vkCmdBindVertexBuffers` 调用**：顶点数据全部来自着色器里的常量数组
- **没有 `vkCmdPushConstants` 调用**：管线布局里没有推送常量范围
- **`vkCmdDraw` 绘制 6 个顶点**：`vertexCount = 6`、`instanceCount = 1`、`firstVertex = 0`、`firstInstance = 0`
- **一个光源只画一个广告牌**：`lightPosition` 是单值，六顶点构成一张方形

## 点光源着色器

### 顶点着色器

```glsl
#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor; // w is intensity
    vec3 lightPosition;
    vec4 lightColor;
} ubo;
```

- **没有 `layout(location = N) in ...` 顶点输入声明**：与清空的顶点输入描述相呼应
- **`OFFSETS` 是长度为 6 的 `const` 数组**：全部编译期常量，不占用描述符或缓冲
- **只有 `fragOffset` 一个输出**：`location = 0`，类型是 `vec2`
- **UBO 块与场景着色器同名同布局**：五个字段的类型与顺序一致
- **`ubo` 只读取 `projection`、`view`、`lightPosition`**：`ambientLightColor` 与 `lightColor` 由片元阶段使用

### 片元着色器

```glsl
layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis >= 1.0) {
        discard;
    }

    outColor = vec4(ubo.lightColor.xyz, 1.0);
}
```

- **`fragOffset` 的 `location` 与顶点输出对应**：都是 0
- **`dot(fragOffset, fragOffset)` 是到中心的距离平方**：`sqrt` 之后得到单位方形内的半径
- **`dis >= 1.0` 的片段被 `discard`**：方形四角落在单位圆外，被裁掉后留下圆形
- **`discard` 发生在深度写入之前**：被丢弃的片段不会写入深度
- **颜色取自 `ubo.lightColor.xyz`**：广告牌显示光源本身的颜色
- **alpha 固定为 1.0**：`colorBlendAttachment.blendEnable` 为 `VK_FALSE`，输出不参与混合
- **不受光照影响**：这里不计算法线与漫反射，直接输出纯色

## 渲染顺序

帧循环里两个渲染系统在同一渲染通道内依次执行：

```cpp
SimpleRenderSystem renderSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
PointLightSystem pointLightSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};
```

```cpp
renderer.beginSwapChainRenderPass(commandBuffer);
renderSystem.renderGameObjects(frameInfo);
pointLightSystem.render(frameInfo);
renderer.endSwapChainRenderPass(commandBuffer);
```

- **两个系统构造在同一处**：都接收 `globalSetLayout->getDescriptorSetLayout()`
- **两个系统共用一份全局描述符集**：`frameInfo.globalDescriptorSet` 被两者各绑定一次
- **`pointLightSystem.render` 在 `renderGameObjects` 之后**：广告牌在场景几何之后提交
- **两者都在 `beginSwapChainRenderPass` 与 `endSwapChainRenderPass` 之间**：属于同一个渲染通道
- **深度测试决定遮挡关系**：广告牌按 `VK_COMPARE_OP_LESS` 比较，被场景几何挡住的片段不会显示
