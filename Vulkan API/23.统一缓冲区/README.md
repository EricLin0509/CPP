# 统一缓冲区

在之前的实现中，我们设计的推送常量结构是这样

```cpp
struct SimplePushConstantsData {
  glm::mat4 transform{1.f};
  glm::mat4 normalMatrix{1.f};
};
```

但这里遇到了瓶颈，就是推送常量的大小只能保证有 128 字节，只能塞下 2 个 `glm::mat4` 矩阵

为了解决这个问题，我们就要使用统一缓冲区来推送更多数据

统一缓冲区 (Uniform Buffer) 是一种用于存储和传递数据的缓冲区，它在渲染过程中被频繁使用

它把着色器所需的"全局常量"（如视图/投影矩阵、光照参数、材质属性等）放在一块 GPU 可访问的显存中, 所有顶点和片元在绘制时读取同一份数据，因此得名 uniform

## 与推送常量、顶点缓冲的区别

| | 推送常量 (Push Constants) | 统一缓冲区 (Uniform Buffer) | 顶点缓冲 (Vertex Buffer) |
| :---: | :---: | :---: | :---: |
| 数据含义 | 整次绘制共享的少量常量 | 整次绘制共享的常量 (矩阵/光照参数) | 每个顶点各不相同的属性 (position/color/normal/uv) |
| 数据位置 | 直接写入命令缓冲区 | 独立的 GPU 缓冲区 | 独立的 GPU 缓冲区 |
| 使用标志 | 无，不占用缓冲区 | `VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT` | `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT` |
| 绑定方式 | `vkCmdPushConstants`，无需描述符集 | `vkCmdBindDescriptorSets`，通过描述符集绑定 | `vkCmdBindVertexBuffers`，配合顶点输入状态 |
| 着色器访问 | `layout(push_constant) uniform` 块 | `layout(set = 0, binding = 0) uniform` 块 | `layout(location = N) in` 输入变量，逐顶点取值 |
| 容量 | 很小 (只能保证有 128 字节) | 大得多，可容纳完整矩阵/数组 | 只受显存限制，可存放整个模型 |
| 内存位置 | 无 | 常驻 `HOST_VISIBLE`，CPU 每帧写入 | 通常 `DEVICE_LOCAL`，staging buffer 上传后不再改动 |
| 更新开销 | 每次绘制都要重新推送 | 一次写入可被多次绘制共享 | 模型加载时一次性上传，基本静态 |
| 访问/更新速度 | 快速 | 慢 (需要经过 PCIE 总线传输) | 读取快，更新需重新上传 |
| 索引配合 | 无 | 无 | 常与索引缓冲 (Index Buffer) 一起使用 |

- 根据 Vulkan 规范，统一缓冲区的大小在桌面端通常是64KB，在移动端通常是16KB

## 内存对齐

统一缓冲区按 std140 布局规则排布，每个成员的起始偏移必须是其对齐值的倍数

- 标量 (`float`/`int`/`bool`) 与 `vec2` 的对齐值是 4 与 8
- `vec3` 与 `vec4` 的对齐值是 16
- `mat4` 的对齐值是 16，大小为 64 字节
- 结构体本身的起始偏移按 16 对齐，总大小向上取整到 16 的倍数

假设现在的统一缓冲区结构是这样

```cpp
struct Ubo {
    glm::mat4 projectionMatrix;
    glm::vec3 directionToLight;
};
```

- `projectionMatrix` 占偏移 `[0, 64)`
- `directionToLight` 的对齐值是 16，`64` 已经是 16 的倍数，所以紧接在偏移 64 处，占 `[64, 76)`
- 结构体总大小向上取整到 16 的倍数，`76 → 80`

那么整个统一缓冲区的样子就是

```
uniformBuffer = [ubo0, padding, ubo1, padding, ubo2, padding, ...]
```

- `uboX`: 76 字节
- `padding`: 4 字节
- `uboX` + `padding` = 80 字节

## 缓冲区封装

`MyBuffer` 把创建、映射、写入、刷新的整套流程收进一个类：

```cpp
MyBuffer(
    VulkanDevice& device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment = 1);
```

- **`instanceSize` 是单个实例的字节数**：顶点缓冲传 `sizeof(Vertex)`，索引缓冲传 `sizeof(uint32_t)`，统一缓冲传 `sizeof(GlobalUbo)`
- **`instanceCount` 是实例个数**：顶点数、索引数，或统一缓冲的 1
- **`minOffsetAlignment` 默认 1**：只有需要把多份实例连续排布、并按实例取偏移时才要传入设备的最小偏移对齐值
- **构造即创建**：构造函数里先算 `alignmentSize` 与 `bufferSize`，再调用 `device.createBuffer`
- **析构即释放**：析构函数先 `unmap()`，再 `vkDestroyBuffer` 与 `vkFreeMemory`，句柄不需要手工管理
- **拷贝构造与拷贝赋值被删除**：缓冲区句柄是唯一的，复制会造成重复释放

对齐值的计算用位运算完成：

```cpp
VkDeviceSize MyBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}
```

- **本质是向上取整到 `minOffsetAlignment` 的倍数**：先加 `min - 1`，再把低位清零
- **按位与要求 `minOffsetAlignment` 是 2 的幂**：`minUniformBufferOffsetAlignment` 在常见设备上是 64，满足这个前提
- **`minOffsetAlignment` 为 0 时原样返回**：避免 `~0` 把结果清成 0
- **`alignmentSize * instanceCount` 得到总大小**：`bufferSize` 因此包含了实例之间的空隙
- **当前所有调用点都用默认值 1**：于是 `alignmentSize == instanceSize`，`bufferSize` 就是"单个实例大小 × 数量"

对外暴露的操作分成两组：

```cpp
VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
void unmap();

void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

void writeToIndex(void* data, int index);
VkResult flushIndex(int index);
VkDescriptorBufferInfo descriptorInfoForIndex(int index);
VkResult invalidateIndex(int index);
```

- **`map` 把内存映射到 `mapped` 成员**：`vkMapMemory` 成功后 `mapped` 指向映射区，`unmap` 负责把它置回 `nullptr`
- **`writeToBuffer` 要求先 `map`**：函数开头有 `assert(mapped && ...)`，未映射时直接断言失败
- **`size` 为 `VK_WHOLE_SIZE` 时整块覆盖**：`memcpy(mapped, data, bufferSize)`，因此传入的 `data` 至少要 `bufferSize` 字节
- **带 `offset` 时先偏移指针再拷贝**：`char* memOffset = (char*)mapped; memOffset += offset;`
- **`flush` 与 `invalidate` 只在非 `HOST_COHERENT` 内存上必要**：前者让 CPU 写入对设备可见，后者让设备写入对 CPU 可见
- **`descriptorInfo` 只是组装一个结构**：`VkDescriptorBufferInfo{buffer, offset, size}`，供描述符集写入时使用
- **`*Index` 系列以 `index * alignmentSize` 为偏移**：同一个缓冲区里排列多份实例时，按下标定位每一份
- **`writeToIndex` 的拷贝长度是 `instanceSize`**：只写一份实例，不会碰到相邻实例的数据
- **当前代码只用到了 `map`、`writeToBuffer`、`flush` 与 `getBuffer`**：`invalidate`、`descriptorInfo` 与 `*Index` 系列尚未被调用

读取句柄与属性的接口都是一行内联函数：

```cpp
VkBuffer getBuffer() const { return buffer; }
void* getMappedMemory() const { return mapped; }
uint32_t getInstanceCount() const { return instanceCount; }
VkDeviceSize getInstanceSize() const { return instanceSize; }
VkDeviceSize getAlignmentSize() const { return instanceSize; }
VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
VkDeviceSize getBufferSize() const { return bufferSize; }
```

- **`getBuffer()` 返回裸句柄**：`vkCmdBindVertexBuffers`、`vkCmdCopyBuffer` 这类命令需要的就是它
- **`getBufferSize()` 返回的是含对齐空隙的总大小**：与 `instanceSize * instanceCount` 不一定相等
- **`getAlignmentSize()` 返回的是 `instanceSize`**：不是成员 `alignmentSize`，两者在 `minOffsetAlignment > 1` 时会不同
- **`getMappedMemory()` 暴露映射指针**：外部可以直接读写，绕过 `writeToBuffer` 的整块拷贝

## 模型缓冲区

模型的顶点缓冲与索引缓冲由 `MyBuffer` 持有：

```cpp
std::unique_ptr<MyBuffer> vertexBuffer;
std::unique_ptr<MyBuffer> indexBuffer;
```

创建顶点缓冲时，暂存缓冲也是 `MyBuffer`：

```cpp
MyBuffer stagingBuffer{
    device,
    vertexSize,
    vertexCount,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
};

stagingBuffer.map();
stagingBuffer.writeToBuffer((void *)vertices.data());

vertexBuffer = std::make_unique<MyBuffer>(
    device,
    vertexSize,
    vertexCount,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
```

- **暂存缓冲是栈上对象**：`createVertexBuffer` 返回时析构函数自动释放它，`vkDestroyBuffer` 与 `vkFreeMemory` 都在析构里完成
- **暂存缓冲带 `HOST_COHERENT`**：CPU 写入对设备立即可见，因此这里没有 `flush` 调用
- **目标缓冲是 `DEVICE_LOCAL`**：位于显存，只带 `TRANSFER_DST_BIT` 与 `VERTEX_BUFFER_BIT`
- **`instanceSize` 与 `instanceCount` 直接来自顶点数组**：`vertexSize = sizeof(Vertex)`、`vertexCount = vertices.size()`
- **`copyBuffer` 的两个句柄都来自 `getBuffer()`**：拷贝长度用的是局部变量 `bufferSize`，即 `vertexCount * sizeof(Vertex)`
- **索引缓冲的处理完全相同**：`indexSize = sizeof(uint32_t)`、`indexCount = indices.size()`，目标缓冲用 `VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT`

绑定与绘制时从 `MyBuffer` 取句柄：

```cpp
VkBuffer buffers[] = {vertexBuffer->getBuffer()};
VkDeviceSize offsets[] = {0};
vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

if (hasIndexBuffer)
{
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}
```

- **`unique_ptr` 随 `MyModel` 一起销毁**：模型释放时缓冲区句柄由 `MyBuffer` 的析构函数回收
- **`MyModel` 只持有两个智能指针与两个计数**：`VkBuffer` 与 `VkDeviceMemory` 都封装在 `MyBuffer` 内部
- **`getBuffer()` 返回的句柄在 `MyBuffer` 生命周期内有效**：绑定命令只是把这个句柄记进命令缓冲区，不复制数据

## 全局统一缓冲区

`my_app.cpp` 里定义了全局统一缓冲区的数据结构：

```cpp
struct GlobalUbo {
  glm::mat4 ProjectionMatrix{1.f};
  glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
};
```

- **`ProjectionMatrix` 是 `P * V`**：投影矩阵与视图矩阵的乘积，相机参数每帧都会变，所以它必须逐帧更新
- **不含模型矩阵 `M`**：`M` 每个物体都不一样，通过推送常量传递
- **字段名首字母大写**：与 `TransformComponent` 里的小写成员区分开
- **`ProjectionMatrix{1.f}` 初始化为单位阵**：单参数构造把对角线填成 1
- **`lightDirection` 带默认值**：`normalize(vec3(1.f, -3.f, -1.f))` 与顶点着色器里的 `DIRECTION_TO_LIGHT` 指向同一个方向
- **默认成员初始化器在每次构造时执行**：`GlobalUbo ubo{}` 每个帧循环都重新构造，`normalize` 也随之每帧执行一次
- **两个字段的偏移是 0 与 64**：实测 `sizeof(GlobalUbo)` 为 76、`alignof` 为 4，与「内存对齐」一节的 `Ubo` 布局完全一致
- **C++ 侧 76 字节，着色器侧 80 字节**：std140 会把结构体尾部补到 16 的倍数，所以描述符的 `range` 需要覆盖 80 字节才能让着色器读到完整的 `lightDirection`

## 每帧缓冲区

统一缓冲区在 `run()` 开头按飞行帧数创建：

```cpp
std::vector<std::unique_ptr<MyBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
for (int i = 0; i < uboBuffers.size(); i++) {
  uboBuffers[i] = std::make_unique<MyBuffer>(
    device,
    sizeof(GlobalUbo),
    1,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  uboBuffers[i]->map();
}
```

- **数量是 `MAX_FRAMES_IN_FLIGHT`**：该常量为 2，与命令缓冲区、信号量、栅栏的数量一致
- **每帧一个缓冲区**：第 N 帧写第 N 个，GPU 可能还在读第 N-1 帧的那份，同一块内存被同时读写会撕裂数据
- **`instanceCount = 1`**：每个缓冲区只放一份 `GlobalUbo`，不是把多份实例塞进一个缓冲区
- **`usageFlags` 是 `VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT`**：这个标志决定缓冲区可以作为描述符的 uniform 类型绑定
- **内存属性只有 `HOST_VISIBLE`，没有 `HOST_COHERENT`**：CPU 写入后不会自动对设备可见，必须显式 flush
- **`map()` 在初始化时调用一次**：之后不再 `unmap`，整块内存保持持久映射，每帧只做 `memcpy`
- **`sizeof(GlobalUbo)` 作为 `instanceSize`**：构造时据此算出 `bufferSize` 并立即创建缓冲区

每帧在录制命令缓冲区之前更新数据：

```cpp
GlobalUbo ubo{};
ubo.ProjectionMatrix = camera.getProjectionMatrix() * camera.getViewMatrix();
uboBuffers[frameIndex]->writeToBuffer(&ubo);
uboBuffers[frameIndex]->flush();
```

- **`GlobalUbo ubo{}` 是局部变量**：每帧构造一个新的，`lightDirection` 由默认成员初始化器填入
- **只赋值 `ProjectionMatrix`**：`lightDirection` 保持默认值，着色器读到的就是同一个方向
- **`frameIndex` 来自 `renderer.getCurrentFrameIndex()`**：与当前正在录制的命令缓冲区同索引
- **`writeToBuffer(&ubo)` 不带 size**：默认 `VK_WHOLE_SIZE`，`memcpy` 覆盖整个 `bufferSize`，即 76 字节
- **随后 `flush()` 不带参数**：默认 `size = VK_WHOLE_SIZE`、`offset = 0`，把整块映射内存刷到设备可见
- **写入发生在 `beginSwapChainRenderPass` 之前**：此时命令缓冲区还没开始渲染，数据已经准备好
- **这行更新与相机更新同处一个循环**：`setViewPosition` 之后、`getProjectionMatrix` 之前，用的是本帧最新的相机状态

统一缓冲区已写入，但着色器还没有从它读取：

- **`pipelineLayoutInfo.setLayoutCount` 是 0**：`pSetLayouts` 为 `nullptr`，管线布局里没有描述符集
- **着色器声明的是 `layout(push_constant) uniform Push`**：`transform` 与 `modelMatrix` 两块矩阵逐物体推送
- **`descriptorInfo()` 尚未被调用**：它返回的 `VkDescriptorBufferInfo` 正是写描述符集时需要的结构
- **因此 `uboBuffers` 目前只承担"每帧上传一份 `P * V`"的动作**：数据写进了显存，还没有和着色器的绑定槽连起来