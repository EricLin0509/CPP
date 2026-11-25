# Alpha 混合 & 透明度

## 透明度（Transparency）

颜色通常写成 `RGBA` 四分量，其中 `A`（Alpha）表示不透明度：

- `A = 1.0` 完全不透明
- `A = 0.0` 完全透明
- 介于两者之间则是半透明

透明度本身只是一个“数值”，它并不会自动产生半透明效果。要让背后的物体“透出来”，还需要一步 **Alpha 混合**。

## Alpha 混合（Alpha Blending）

Alpha 混合是渲染管线末端的“颜色合成”步骤：把新片元（源 src）和帧缓冲里已有的像素（目标 dst）按权重叠加。最常用的公式是：

```
dst = src.rgb * src.a + dst.rgb * (1 - src.a)
```

也就是常见的 `SRC_ALPHA` / `ONE_MINUS_SRC_ALPHA` 组合。直观理解：

- 源越不透明（`src.a` 越大），越接近源颜色
- 源越透明（`src.a` 越小），越接近已有颜色

| 字段 | 含义 | 场景管线 | 点光源管线 |
| --- | --- | --- | --- |
| `blendEnable` | 是否开启混合 | `VK_FALSE` | `VK_TRUE` |
| `colorWriteMask` | 写入哪些通道 | `R`、`G`、`B`、`A` 四位全开 | `R`、`G`、`B`、`A` 四位全开 |
| `srcColorBlendFactor` | 源颜色权重 | `ONE` | `SRC_ALPHA` |
| `dstColorBlendFactor` | 目标颜色权重 | `ZERO` | `ONE_MINUS_SRC_ALPHA` |
| `colorBlendOp` | 颜色运算 | `ADD` | `ADD` |
| `srcAlphaBlendFactor` | Alpha 通道源权重 | `ONE` | `ONE` |
| `dstAlphaBlendFactor` | Alpha 通道目标权重 | `ZERO` | `ZERO` |
| `alphaBlendOp` | Alpha 运算 | `ADD` | `ADD` |

- **颜色通道与 Alpha 通道各用一套权重**：`srcColorBlendFactor`/`dstColorBlendFactor` 作用于 `RGB`，`srcAlphaBlendFactor`/`dstAlphaBlendFactor` 作用于 `A`
- **点光源管线的 Alpha 通道权重是 `ONE`/`ZERO`**：Alpha 分量原样写入，不参与混合运算
- **两条管线的 `colorWriteMask` 相同**：`R`、`G`、`B`、`A` 四个通道都允许写入
- **`colorBlendInfo.pAttachments` 指向 `configInfo.colorBlendAttachment`**：混合字段写在被指向的那份结构上，`vkCreateGraphicsPipelines` 时才会读到

## 片元着色器的透明度输出

`point_light.frag` 按片段到圆心的距离给出 Alpha：

```glsl
const float M_PI = 3.14159265358;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));
    if (dis >= 1.0) {
        discard;
    }

    float cosDis = 0.5 * (cos(dis * M_PI) + 1.0); // ranges from 1 -> 0
    outColor = vec4(push.color.xyz + cosDis, cosDis);
}
```

- **`M_PI` 是 `const float` 常量**：值为 `3.14159265358`
- **`dis` 是片段到广告牌中心的归一化距离**：`sqrt(dot(fragOffset, fragOffset))`，圆心处为 0、圆周处为 1
- **`cosDis` 是余弦衰减**：`0.5 * (cos(dis * π) + 1.0)`，`dis = 0` 时为 1、`dis = 1` 时为 0，中间单调递减
- **`dis >= 1.0` 的片段仍然被 `discard`**：圆形边界之外的片段不产生任何输出
- **Alpha 分量直接取 `cosDis`**：圆心完全不透明，向边缘逐渐变为完全透明
- **RGB 分量是 `push.color.xyz + cosDis`**：每个通道都在光源颜色上叠加同一个标量，圆心处最亮
- **叠加量与 Alpha 同源**：`cosDis` 同时充当亮度增量与不透明度，越靠中心越亮也越不透明
- **输出的是 `vec4`**：前三个分量是颜色、第四个分量是 Alpha

## 启用混合的管线配置

`MyPipeline` 提供一个静态函数改写混合字段：

```cpp
void MyPipeline::enableAlphaBlending(PipelineConfigInfo& configInfo)
{
  configInfo.colorBlendAttachment.blendEnable = VK_TRUE;

  configInfo.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;
  configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}
```

- **参数是 `PipelineConfigInfo&`**：就地改写调用方的那份配置，没有返回值
- **`blendEnable` 置为 `VK_TRUE`**：开启该颜色附件的混合
- **`srcColorBlendFactor` 为 `SRC_ALPHA`**：源颜色按自身 Alpha 加权
- **`dstColorBlendFactor` 为 `ONE_MINUS_SRC_ALPHA`**：目标颜色按 `1 - src.a` 加权
- **`colorBlendOp` 与 `alphaBlendOp` 都是 `VK_BLEND_OP_ADD`**：两组权重各自相加
- **Alpha 通道的权重是 `ONE` 与 `ZERO`**：混合后的 Alpha 等于源的 Alpha
- **`colorWriteMask` 四个通道全开**：混合结果写入全部通道

调用位置在 `defaultConfigInfo` 之后：

```cpp
PipelineConfigInfo pipelineConfigInfo{};
MyPipeline::defaultConfigInfo(pipelineConfigInfo);
MyPipeline::enableAlphaBlending(pipelineConfigInfo);
pipelineConfigInfo.bindingDescriptions.clear();
pipelineConfigInfo.attributeDescriptions.clear();
pipelineConfigInfo.renderPass = renderPass;
pipelineConfigInfo.pipelineLayout = pipelineLayout;
```

- **只对点光源管线调用**：`PointLightSystem::createPipeline` 里执行这一行，`SimpleRenderSystem` 的配置不经过这个函数
- **顺序是先默认值再改写**：`defaultConfigInfo` 先把 `blendEnable` 设为 `VK_FALSE`，紧接着 `enableAlphaBlending` 把它设为 `VK_TRUE`
- **改写发生在清空顶点描述之前**：混合字段与顶点输入描述互不影响
- **`renderPass` 与 `pipelineLayout` 仍在最后指定**：混合字段属于 `colorBlendAttachment`，与这两项各自独立

## 按距离排序绘制光源

`PointLightSystem::render` 先把光源按到相机的距离排序，再逆序绘制：

```cpp
// sort lights
std::map<float, GameObject::id_t> sortedLights;
for (auto& kv : frameInfo.gameObjects) {
  auto& obj = kv.second;
  if (obj.pointLight == nullptr) continue;

  // calculate distance to camera
  auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
  float disSquared = glm::dot(offset, offset);
  sortedLights[disSquared] = obj.getID();
}
```

- **容器是 `std::map<float, GameObject::id_t>`**：键为距离平方，值为对象 `id`
- **`<map>` 被包含**：`point_light_system.cpp` 的头文件列表里有 `<map>`
- **键是距离平方而非距离**：`glm::dot(offset, offset)` 省掉一次开方，排序结果不受影响
- **`offset` 由相机位置减去光源位置得到**：`frameInfo.camera.getPosition()` 是世界空间坐标
- **`std::map` 按键升序排列**：插入完成后迭代顺序就是从近到远
- **键相同的两盏灯会互相覆盖**：`std::map` 不允许重复键，距离平方相等的后写入者保留
- **只收录 `pointLight` 非空的对象**：场景模型不进入这张表
- **排序发生在 `pipeline->bind` 之前**：先算完表，再绑定管线与描述符集

绘制时逆序遍历：

```cpp
// iterate through sorted lights in reverse order
for (auto it = sortedLights.rbegin(); it != sortedLights.rend(); it++)
{
  auto& obj = frameInfo.gameObjects.at(it->second);
  ...
  vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}
```

- **逆序即从远到近**：`rbegin()` 到 `rend()` 走的是键从大到小的方向
- **`at(it->second)` 按 `id` 取回对象**：表中只存 `id`，绘制时再回到 `gameObjects` 查
- **每个光源一次推送加一次绘制**：`vkCmdPushConstants` 与 `vkCmdDraw(6, 1, 0, 0)` 成对出现
- **深度写入是 `VK_TRUE`**：点光源管线的 `depthWriteEnable` 来自 `defaultConfigInfo`，混合后的颜色照样写进深度附件

## 场景与光源的提交顺序

帧循环里场景模型先提交、光源后提交，中间留有空行并附注释：

```cpp
renderer.beginSwapChainRenderPass(commandBuffer);

// order here matters
renderSystem.renderGameObjects(frameInfo);
pointLightSystem.render(frameInfo);

renderer.endSwapChainRenderPass(commandBuffer);
```

- **`renderGameObjects` 在前**：不透明几何体先写入颜色附件与深度附件
- **`pointLightSystem.render` 在后**：混合发生在已有颜色的基础上，目标颜色就是场景已经写好的像素
- **两次调用之间在同一渲染通道内**：都在 `beginSwapChainRenderPass` 与 `endSwapChainRenderPass` 之间
- **注释写明顺序有影响**：源码里就写着 `// order here matters`

## 相机位置访问器

`MyCamera` 带一个取相机世界坐标的访问器：

```cpp
const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }
```

- **从 `inverseViewMatrix` 取第四列**：`[3]` 是列下标，前三个分量是相机世界坐标
- **返回值是 `glm::vec3`**：把 `vec4` 的前三个分量取出来
- **函数是 `const`**：可在 `const MyCamera` 上调用
- **逆视图矩阵由 `setViewDirection` 与 `setViewPosition` 写入**：`inverseViewMatrix[3]` 的三个分量填的是传入的 `position`
- **调用点是光源排序**：`frameInfo.camera.getPosition()` 与光源位置相减得到距离向量

## 配置结构的构造与拷贝控制

`PipelineConfigInfo` 显式声明了构造函数与拷贝控制：

```cpp
struct PipelineConfigInfo {
  PipelineConfigInfo() = default;
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

  std::vector<VkVertexInputBindingDescription> bindingDescriptions;
  ...
};
```

- **默认构造用 `= default`**：`PipelineConfigInfo pipelineConfigInfo{};` 仍然可用
- **拷贝构造与拷贝赋值被 `delete`**：对象不可按值传递，也不可赋值
- **成员里含指向自身成员的指针**：`colorBlendInfo.pAttachments` 指向 `configInfo.colorBlendAttachment`，`dynamicStateInfo.pDynamicStates` 指向 `configInfo.dynamicStates`
- **按位拷贝会让指针仍指向原对象**：副本的 `pAttachments` 会悬空或指向别人的数据，因此拷贝被显式禁止
- **`enableAlphaBlending` 与 `defaultConfigInfo` 都接收引用**：`PipelineConfigInfo&` 让改写落在调用方那一份上
