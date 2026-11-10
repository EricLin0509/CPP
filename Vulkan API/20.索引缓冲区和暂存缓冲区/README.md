# 索引缓冲区 & 暂存缓冲区

## 索引缓冲区

索引缓冲区 (Index Buffer) 是一种专门用来存储顶点索引的缓冲区

目前的实现中，我们是逐个三角形传入顶点数据来构建 3D 模型的

但这样一来，绘制一个矩形面就会有两个顶点被重复存储，造成内存浪费

为此，可以用索引缓冲区避免顶点被重复存储，从而降低内存占用

以矩形为例，可以按下面的方式传入数据：

1. 传入 4 个互不相同的顶点
2. 再传入一个由 6 个整数构成的索引数组，例如 `[0, 1, 2, 2, 3, 0]`

按这组索引绘制出的矩形如下：

```
1 ---- 0
|     /|
|    / |
|   /  |
|  /   |
| /    |
|/     |
2 ---- 3
```

### 模型构建器

`MyModel` 用一个 `Builder` 把顶点和索引一起传进来：

```cpp
class MyModel {
    public:
        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
        };

        MyModel(VulkanDevice& device, const Builder& builder);

    private:
        bool hasIndexBuffer = false;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        uint32_t indexCount;

        void createVertexBuffer(const std::vector<Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);
};
```

- **两个数组分开存放**：`vertices` 存顶点数据，`indices` 存 `uint32_t` 索引，两者长度不必相等
- **构造函数一次建两块缓冲区**：`createVertexBuffer(builder.vertices)` 之后紧跟 `createIndexBuffer(builder.indices)`
- **`indexCount` 与 `vertexCount` 并列**：绘制时用哪个计数，取决于有没有索引
- **`hasIndexBuffer` 是运行时的开关**：它决定是否绑定索引缓冲、是否用索引绘制、析构时是否释放索引缓冲的内存

### 创建索引缓冲区

```cpp
void MyModel::createIndexBuffer(const std::vector<uint32_t>& indices)
{
    indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = indexCount > 0;
    if (!hasIndexBuffer) return;

    VkDeviceSize bufferSize = indexCount * sizeof(uint32_t);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device.device(), stagingBufferMemory);

    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexBufferMemory);

    device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
    vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
}
```

- **空索引直接返回**：`indexCount == 0` 时 `hasIndexBuffer` 保持 `false`，既不创建缓冲区也不占用显存
- **`sizeof(uint32_t)` 而非 `sizeof(Vertex)`**：缓冲区大小由索引个数乘 4 字节决定，与顶点大小无关
- **`VK_BUFFER_USAGE_INDEX_BUFFER_BIT`**：声明这块缓冲区将作为索引缓冲使用，与顶点缓冲的用途标志不同
- **`VK_BUFFER_USAGE_TRANSFER_DST_BIT`**：声明它同时是复制操作的目标，`vkCmdCopyBuffer` 才能往这块缓冲区写入
- **索引缓冲与顶点缓冲结构对称**：同样的暂存 → 复制 → 销毁流程，只是数据源与用途标志换了

#### 没有索引时的回退

```cpp
indexCount = static_cast<uint32_t>(indices.size());
hasIndexBuffer = indexCount > 0;
if (!hasIndexBuffer) return;
```

- **`hasIndexBuffer` 由数据决定，而非编译期常量**：同一个 `MyModel` 类既能表示带索引的网格，也能表示纯顶点列表
- **析构函数同样判断**：`if (hasIndexBuffer)` 成立才销毁索引缓冲与内存，未创建过的句柄不会被误释放
- **两条绘制路径共存**：`vkCmdDrawIndexed` 与 `vkCmdDraw` 二选一，调用方不需要知道模型内部有没有索引

### 绑定与绘制

```cpp
void MyModel::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (hasIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void MyModel::draw(VkCommandBuffer commandBuffer)
{
    if (hasIndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}
```

- **顶点缓冲仍要绑定**：索引只是"取第几个顶点"，顶点数据本身仍要绑定在 binding 0 上
- **`VK_INDEX_TYPE_UINT32` 与 `uint32_t` 对应**：索引数组的元素类型必须是 `uint32_t`，否则类型与数据宽度不匹配
- **`vkCmdDrawIndexed` 的参数**：`indexCount` 是要画的索引个数、第二个 `1` 是实例数、三个 `0` 分别是首个索引偏移、顶点偏移和首个实例号
- **`vkCmdDraw` 只数顶点**：没有索引时按 `vertexCount` 顺序依次取顶点
- **`bind` 里索引绑定放在顶点绑定之后**：两条命令互不依赖，顺序只为阅读方便

### 立方体的索引数据

`createCubeModel` 先用 `Builder` 收集数据，再统一传入构造函数：

```cpp
MyModel::Builder modelBuilder{};
modelBuilder.vertices = {
    // left face (white)
    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
    {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
    {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
    // ... 其余五个面各 4 个顶点 ...
};
for (auto& v : modelBuilder.vertices) {
    v.position += offset;
}

modelBuilder.indices = {0, 1, 2, 0, 3, 1, 4, 5, 6, 4, 7, 5, 8, 9, 10, 8, 11, 9,
                        12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

return std::make_unique<MyModel>(device, modelBuilder);
```

- **每个面只留 4 个顶点**：两个三角形需要 6 个顶点，其中两个被两个三角形共用；只保留互不相同的顶点后剩 4 个
- **索引按面分组、每组 6 个**：第 `f` 个面的索引是 `indices[6f .. 6f+5]`，对应顶点 `vertices[4f .. 4f+3]`
- **每组的模式是 `{0, 1, 2, 0, 3, 1}` (相对组内偏移)**：第一个三角形用 0、1、2，第二个用 0、3、1，两个三角形共用顶点 0
- **`offset` 在构建索引之前加到顶点上**：索引只记下标，顶点整体平移不会改变下标，所以顺序无关紧要

六个面的索引与顶点对应关系：

| 面 | 颜色 | 组内顶点 | 索引 (绝对) |
| :-- | :-- | :-- | :-- |
| left | 白 | 0–3 | `0, 1, 2, 0, 3, 1` |
| right | 黄 | 4–7 | `4, 5, 6, 4, 7, 5` |
| top | 橙 | 8–11 | `8, 9, 10, 8, 11, 9` |
| bottom | 红 | 12–15 | `12, 13, 14, 12, 15, 13` |
| nose | 蓝 | 16–19 | `16, 17, 18, 16, 19, 17` |
| tail | 绿 | 20–23 | `20, 21, 22, 20, 23, 21` |

顶点与索引各自的数据量：

| 项目 | 数值 |
| :-- | :--: |
| 顶点个数 | 24 |
| 索引个数 | 36 |
| 顶点字节 (每个 24 字节) | 576 |
| 索引字节 (每个 4 字节) | 144 |
| 合计 | 720 |

- **顶点个数是 36 个三角形顶点的 2/3**：36 → 24，省掉 12 个重复顶点
- **总字节 720**：索引额外占 144 字节，所以相比 864 字节的纯顶点方案实际只省 144 字节
- **八个角仍各有三份拷贝**：立方体只有 8 个几何角点，但每个角被三个面共享，而三个面的颜色不同——顶点里绑着颜色，颜色不同就不能共用同一个顶点，所以 24 = 8 × 3 已经是这个数据格式的下限
- **索引缓冲的收益随面数增长**：一个面用 6 个顶点画 2 个三角形只省 2 个顶点，若一个网格有几十个三角形共享同一批顶点，节省比例会远高于此处的 33%

## 暂存缓冲区

此前的实现中，用于存放 3D 模型顶点数据与索引数据的缓冲区，在分配内存时都带有下面这两个标志

```cpp
VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
```

- `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`：内存对主机可见，主机可以直接读写这块内存
- `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`：主机与设备对这块内存的访问保持一致，主机写入后无需手动 flush，设备读取时也无需手动 invalidate

但这样分配并不是最优的做法，因为主机内存的访问速度较慢，设备内存则快得多

因此，顶点数据和索引数据应当存放在设备内存中，而不是主机内存中

而要写入设备内存，就需要借助暂存缓冲区：先把数据写入主机可见的暂存缓冲区，再把它复制到设备内存

### 顶点缓冲区的两步写入

```cpp
void MyModel::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "At least 3 vertices are required to create a triangle.");

    VkDeviceSize bufferSize = vertexCount * sizeof(Vertex);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device.device(), stagingBufferMemory);

    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory);

    device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
    vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
}
```

整段流程可以拆成五步：

| 步骤 | 操作 | 目的 |
| :--: | :-- | :-- |
| 1 | 建一块 `HOST_VISIBLE` 的 `TRANSFER_SRC` 缓冲区 | 让主机能写 |
| 2 | `vkMapMemory` + `memcpy` + `vkUnmapMemory` | 把顶点数据搬进这块内存 |
| 3 | 建一块 `DEVICE_LOCAL` 的 `VERTEX_BUFFER` 缓冲区，同时声明 `TRANSFER_DST` | 最终存放位置，且能被复制写入 |
| 4 | `copyBuffer(staging, vertex, size)` | 设备内部复制，主机不参与 |
| 5 | 销毁暂存缓冲区与内存 | 数据已经在设备内存里，中转站不再需要 |

- **两个缓冲区大小相同**：`bufferSize` 算一次、用两次，复制区域正好覆盖全部数据
- **`vertexCount >= 3` 的断言**：`vkCmdDraw` 至少要一个三角形，顶点不足在创建时就拦下来
- **`memcpy` 的长度用 `static_cast<size_t>`**：`VkDeviceSize` 是 64 位无符号数，`memcpy` 要 `size_t`，显式转换避免告警
- **映射后立即解除**：`vkMapMemory` 与 `vkUnmapMemory` 只夹住一次写入，不需要在整个生命周期里保持映射

#### 用途标志与内存属性

两块缓冲区的参数成对出现，正好互为镜像：

| | 暂存缓冲区 | 最终缓冲区 |
| :--: | :-- | :-- |
| 用途 | `VK_BUFFER_USAGE_TRANSFER_SRC_BIT` | `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT \| VK_BUFFER_USAGE_TRANSFER_DST_BIT` (索引缓冲则是 `INDEX_BUFFER_BIT \| TRANSFER_DST_BIT`) |
| 内存属性 | `HOST_VISIBLE \| HOST_COHERENT` | `DEVICE_LOCAL` |

- **`TRANSFER_SRC_BIT` 声明这块缓冲区会被当作复制来源**：驱动据此决定内存布局，主机写入的数据可以按线性布局摆放
- **`TRANSFER_DST_BIT` 声明这块缓冲区会被当作复制目标**：`vkCmdCopyBuffer` 要求源与目标各自声明对应的用途位，缺了它校验层会报出用法错误
- **源与目标各一个传输位，正好配对**：暂存缓冲区是 `SRC`，最终缓冲区是 `DST`，一次复制把两者串起来
- **`DEVICE_LOCAL` 是显存**：设备访问最快，但主机不可见，`vkMapMemory` 映射不了，所以必须靠复制写入
- **`HOST_COHERENT` 省掉显式同步**：没有这个标志时，`vkUnmapMemory` 前要调用 `vkFlushMappedMemoryRanges` 把 CPU 缓存刷到对设备可见；这里由驱动保证一致
- **不合并成一个标志位组合**：如果直接用 `HOST_VISIBLE | DEVICE_LOCAL` 去建最终缓冲区，能不能成立取决于物理设备有没有这种"可被主机访问的显存"，不可移植

#### 一次性命令缓冲区

`copyBuffer` 把复制动作交给 GPU 执行，它借用的是一条只提交一次的短命命令缓冲区：

```cpp
void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}
```

- **`VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`**：声明这条命令缓冲区只提交一次，驱动可以按最省的方式安排内存
- **`copyRegion` 的三个字段**：源偏移、目标偏移都为 0，`size` 覆盖整块数据，所以是一次完整的拷贝
- **`endSingleTimeCommands` 里 `vkQueueWaitIdle`**：提交后等队列空，`copyBuffer` 返回时复制一定已经完成
- **同步靠"等队列空闲"**：没有使用栅栏或信号量，牺牲一点并行度换来实现简单——模型加载只发生在启动阶段，这点开销可以接受
- **命令缓冲区用完即释放**：`vkFreeCommandBuffers` 与 `beginSingleTimeCommands` 里的 `vkAllocateCommandBuffers` 配对

#### 暂存缓冲区的生命周期

```cpp
device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
```

- **销毁紧跟复制**：`copyBuffer` 返回时队列已空闲，此时销毁暂存缓冲区不会截断还在执行的传输
- **临时句柄是局部变量**：`stagingBuffer`、`stagingBufferMemory` 都是函数内的局部变量，函数结束就离开作用域，不存在"忘记释放"的路径
- **最终缓冲区才是成员**：`vertexBuffer`、`vertexBufferMemory` 保存在对象里，由析构函数负责释放
- **每块缓冲区一次中转**：顶点数据与索引数据各自走一遍完整流程，各自用自己的暂存缓冲区，互不影响

### 析构与索引缓冲

```cpp
MyModel::~MyModel()
{
    vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
    vkFreeMemory(device.device(), vertexBufferMemory, nullptr);

    if (hasIndexBuffer)
    {
        vkDestroyBuffer(device.device(), indexBuffer, nullptr);
        vkFreeMemory(device.device(), indexBufferMemory, nullptr);
    }
}
```

- **顶点缓冲无条件释放**：它一定被创建过
- **索引缓冲有条件释放**：`hasIndexBuffer` 为 `false` 时这两个句柄从未被赋值，销毁它们会传入未初始化值
- **`vkFreeMemory` 紧跟 `vkDestroyBuffer`**：内存依附于缓冲区，先销毁对象再释放内存，顺序不能颠倒
- **暂存缓冲区不在这里出现**：它们在创建函数里就已经释放完毕
