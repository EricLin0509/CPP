# 推送常量

推送常量 (Push Constants) 是 Vulkan 中用于将小量数据传递给着色器的一种机制

不过，推送常量的容量很小，根据 Vulkan 规范规定，Push Constants 只能保证有 128 字节的容量

所以一般只用于传递少量的数据，比如 offset，颜色等数据

## 示例

如果试图把 `mat4 model; mat4 view; mat4 proj`; 全塞进去，结果 64*3 = 192 字节，直接超出了 128 字节的保底限制 (有些显卡可能最多支持 256，但为了跨平台兼容，规范保底只有 128)

所以，对于以下情况：
- 小于 128 字节且高频变化 (如：平移 offset、纯色、时间戳) -> 用 Push Constants
- 大于 128 字节 (如：包含多个矩阵和复杂材质的 UBO) -> 老老实实用 Uniform Buffer (UBO) + Descriptor Set

## 定义推送常量数据结构

在 C++ 端定义与着色器中推送常量块对应的结构体，需要注意内存对齐：

```cpp
struct SimplePushConstantsData {
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};
```

`alignas(16)` 是因为 GLSL 中 `vec3` 占据 16 字节对齐的槽位 (vec3 后面隐含 padding 到 vec4 大小)，C++ 端必须手动对齐以匹配 GLSL 的布局规则

### GLSL std140 布局对齐规则

推送常量块默认使用 `std140` 布局，其核心对齐规则如下：

| 类型 | 对齐要求 (字节) | 占用大小 (字节) |
|------|-----------------|-----------------|
| `scalar` (int, float, bool) | 4 | 4 |
| `vec2` | 8 | 8 |
| `vec3` | **16** | 12 (但占据 16 字节槽位) |
| `vec4` | 16 | 16 |
| `mat2` (列优先) | 8 | 16 (2 个 vec2 列) |
| `mat3` (列优先) | 16 | 48 (3 个 vec4 列，每列 16) |
| `mat4` (列优先) | 16 | 64 (4 个 vec4 列) |
| 数组元素 | 与 vec4 相同，即 16 字节对齐 | 每元素按 16 字节对齐 |

关键要点：

1. **`vec3` 的陷阱**：GLSL 中 `vec3` 的对齐要求是 16 字节 (与 `vec4` 相同)，而非 12 字节
    - 这意味着 `vec3` 后面会隐含 4 字节的 padding
    - 在 C++ 中 `glm::vec3` 只有 12 字节且默认 4 字节对齐
    - 所以必须用 `alignas(16)` 强制对齐
2. **结构体规则**：结构体本身的大小必须是其最大成员对齐值的整数倍
3. **数组规则**：`std140` 布局下，数组的每个元素都按 16 字节对齐，即使元素类型是 `float` 或 `vec2`

以本例中的 `SimplePushConstantsData` 为例，内存布局如下：

```
偏移  0 ~  7: vec2 offset     (8 字节, 8 字节对齐)
偏移  8 ~ 15: (padding)        (8 字节填充, 使 color 满足 16 字节对齐)
偏移 16 ~ 27: vec3 color      (12 字节, 16 字节对齐)
偏移 28 ~ 31: (padding)        (4 字节填充, 使结构体大小为 16 的倍数)
总计: 32 字节
```

如果不加 `alignas(16)`，C++ 端的 `sizeof(SimplePushConstantsData)` 为 20 字节，与 GLSL 端的 32 字节不匹配，会导致着色器读取到错误的数据

## 着色器中声明推送常量

顶点着色器和片段着色器都可以访问同一个推送常量块，使用 `layout(push_constant)` 声明：

```glsl
layout(push_constant) uniform Push {
    vec2 offset;
    vec3 color;
} push;
```

- 顶点着色器中使用 `push.offset` 对顶点位置进行偏移：`gl_Position = vec4(position + push.offset, 0.0, 1.0);`
- 片段着色器中使用 `push.color` 作为输出颜色：`outColor = vec4(push.color, 1.0);`

这样颜色不再依赖顶点属性插值，而是由推送常量直接指定

## 配置管线布局 (Pipeline Layout)

### 配置推送常量范围

在创建管线布局时，需要指定推送常量的范围 (`VkPushConstantRange`)：

```cpp
VkPushConstantRange pushConstantRange{};
pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
pushConstantRange.offset = 0;
pushConstantRange.size = sizeof(SimplePushConstantsData);
```

- `stageFlags`：指定哪些着色器阶段可以访问该推送常量，这里顶点和片段着色器都需要访问
- `offset`：推送常量在推送常量块中的偏移量 (字节)
- `size`：推送常量数据的大小 (字节)

### 在管线布局中配置推送常量范围

然后在 `VkPipelineLayoutCreateInfo` 中添加推送常量个数和指针：

```cpp
pipelineLayoutInfo.pushConstantRangeCount = 1;
pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
```

## 录制命令时推送常量

### 推送常量数据

在录制命令缓冲区时，通过 `vkCmdPushConstants` 将数据推送到着色器：

```cpp
vkCmdPushConstants(commandBuffer,
    pipelineLayout,
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    0,                              // offset
    sizeof(push),                   // size
    &push);                         // 数据指针
```

之后每次调用 `vkCmdPushConstants` ，后续的绘制命令都会使用该推送常量值

因此可以在同一次渲染通道中，通过循环多次推送不同的常量值来绘制多个不同位置、不同颜色的物体：

```cpp
for (int i = 0; i < 4; i++) {
    SimplePushConstantsData push = {};
    push.offset = {0.0f, -0.4f + i * 0.25f};
    push.color = {0.0f, 0.2f + i * 0.2f, 0.3f + i * 0.3f};

    vkCmdPushConstants(..., &push);
    model->draw(commandBuffers[imageIndex]);
}
```

这种方式无需为每个物体创建单独的 uniform buffer，适合传递少量频繁变化的数据

### 为什么推送常量的数据不需要创建 VkBuffer 和 vkMapMemory？

因为 Push Constants 并不分配任何显存空间

当调用 `vkCmdPushConstants` 时，数据实际上是直接被内联（Inline）写进了命令缓冲区（Command Buffer）的指令流中

当 GPU 执行到这条指令时，会直接把数据加载到芯片内部极其靠近计算单元的寄存器（或常量缓存）中

这就是为什么它只有 128 字节，也是为什么它极快的原因——零显存带宽消耗，零描述符集切换
