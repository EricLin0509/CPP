# 顶点缓冲区

顶点缓冲区 (Vertex Buffer) 是用于存储顶点数据的缓冲区，例如顶点位置、颜色、纹理坐标等

之后将这个顶点缓冲区传递给顶点着色器，用于渲染顶点数据

## 顶点缓冲区的作用

现代的3D模型通常由成百上千个三角形组成，如果采用硬编码的方式存储顶点数据，会导致内存占用过大，且不便于3D模型的渲染

于是就引入了顶点缓冲区，用于存储顶点数据，而不是直接在代码中硬编码顶点数据

## 顶点属性

由于顶点缓冲区中存储是一组连续数据，但这样无法知道到底是一个二维坐标还是三维坐标，甚至是一个颜色值还是纹理坐标

所以要引入一个顶点属性描述，用于描述顶点缓冲区中每个顶点的属性

假设顶点缓冲区的内存如下:

```
{a, b, c, d, e, f, g, h, ...}
```

然后引入这样的顶点属性:

```glsl
layout(location = 0) in vec2 position;
```

那些在 GPU 眼中，顶点缓冲区中的数据长这样:

```
{(a, b), (c, d), (e, f), (g, h), ...}
```

- `(a, b)`: 顶点的位置坐标

每两个数据为一个顶点的位置坐标

同样的，也可以同时设置多个顶点属性如颜色属性、纹理坐标属性等

```glsl
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
```

那么在 GPU 眼中，顶点缓冲区中的数据长这样:

```
{[(a, b), (c, d, e)], [(f, g), (h, i, j)]}
```

- `(a, b)`: 顶点的位置坐标
- `(c, d, e)`: 顶点的颜色属性
- `[]`: 整个顶点的样子

也可以每种顶点属性在各自的顶点缓冲区中存储

```
{(a, b), (c, d), (e, f), (g, h), ...} // 顶点的位置坐标
{(a, b, c), (d, e, f), (g, h, i), ...} // 顶点的颜色属性
```

而这就需要顶点绑定描述 (Vertex Binding Description) 来确定那个顶点缓冲区存储的是哪个顶点属性的数据

### 交错 (Interleaved) vs 分离 (Separate) 的性能

在顶点缓冲区中，顶点数据的存储方式有两种

- 交错存储 (Interleaved)：
    - 数据像这样 `[Pos][Color][Pos][Color]...`
    - 优点：GPU 读取数据时，缓存命中率 (Cache Hit) 极高。当读取一个顶点的位置时，它附近的颜色数据已经被顺手拉进了 CPU 的缓存行 (Cache Line) 里

- 分离存储 (Separate)：
    - 数据像这样：一个纯 Position 数组，一个纯 Color 数组
    - 优点：适合动态修改。比如你只要更新颜色，不需要重新上传位置数据
    - 缺点：GPU 需要同时向两块完全不同的内存区域发请求，内存带宽开销变大

- 业界共识：绝大多数情况下，交错存储因为具有极高的内存局部性，性能远胜分离存储

## 顶点绑定描述

顶点绑定描述 (Vertex Binding Description) 用于建立顶点缓冲区与顶点属性之间的关联，并指定从缓冲区中读取顶点数据的方式

它有以下成员

- `binding`: 绑定索引，标识该描述对应哪个顶点缓冲区 (对应 `VkVertexInputBindingDescription` 数组的下标)
- `stride`: 步长，即从一个顶点数据的起始位置到下一个顶点数据的起始位置之间的字节数
- `inputRate`: 顶点输入速率，决定多久从缓冲区中读取一次数据
    - `VK_VERTEX_INPUT_RATE_VERTEX`: 每个顶点读取一次 (逐顶点读取)
    - `VK_VERTEX_INPUT_RATE_INSTANCE`: 每个实例读取一次 (逐实例读取，用于实例化渲染)

顶点绑定描述告诉 GPU 从哪个缓冲区取数据、每次跨过多少字节、以及按顶点还是按实例推进，但它并未说明缓冲区内部单个顶点数据的具体布局，这需要顶点属性描述 (Vertex Attribute Description) 来进一步定义

## 顶点属性描述

顶点属性描述 (Vertex Attribute Description) 用于定义单个顶点内各属性的具体位置和格式，告诉 GPU 如何从顶点数据中解析出每一个属性

它有以下成员

- `binding`: 绑定索引，标识该属性从哪个顶点缓冲区中获取数据 (与顶点绑定描述的 `binding` 对应)
- `location`: 属性位置索引，对应顶点着色器中 `layout(location = N)` 的 `N`
- `format`: 属性的数据格式
    - `VK_FORMAT_R32G32_SFLOAT` 对应 `vec2`
    - `VK_FORMAT_R32G32B32_SFLOAT` 对应 `vec3`
- `offset`: 属性偏移量，即该属性在单个顶点数据块中的起始字节偏移

## 代码实现

### 之前的痛点

由于之前的顶点数据是**硬编码在着色器里**的，GPU 通过 `gl_VertexIndex` 从着色器内部的数组中取顶点位置

但这样不易于动态修改顶点数据

现在我们把顶点数据从着色器中**搬到 CPU 端的顶点缓冲区**，再上传到 GPU，实现了数据与逻辑的分离

### 着色器变更

#### 顶点着色器

之前我们的着色器文件有 `ver2 position[3]` 这种硬编码

但现在就不需要了，直接使用 `layout(location = 0) in vec2 position;` 接收顶点缓冲区中的顶点数据

```glsl
#version 450

layout(location = 0) in vec2 position;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
```

- `layout()`: 用于指定顶点属性的布局
- `location`: 该属性在顶点缓冲区中的位置索引
- `in`: 表示该属性是输入属性，从顶点缓冲区中读取

### 顶点结构体

```cpp
struct Vertex {
    glm::vec2 position;
    
    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};
```

- `position`：顶点的二维位置坐标
- `getBindingDescriptions()`：返回顶点绑定描述，告诉 GPU 如何从缓冲区中逐顶点读取数据
- `getAttributeDescriptions()`：返回顶点属性描述，告诉 GPU 如何解析单个顶点内的每个属性

#### 顶点绑定描述实现

```cpp
std::vector<VkVertexInputBindingDescription> MyModel::Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}
```

- `binding = 0`：绑定到第 0 个顶点缓冲区
- `stride = sizeof(Vertex)`：每个顶点占用的字节数，GPU 每读完一个顶点，跳过 `stride` 字节到下一个顶点
    - **注意**: 由于C++结构体为了 CPU 读取效率，会自动填充字节，可能会导致偏移量计算错误
    - 所以，建议使用 `offsetof(Vertex, 属性名)` 来自动计算偏移量，不要手动去数字节数
    - 这里使用 `sizeof` 是因为该结构体里面只有 `position` 一个属性
- `inputRate = VK_VERTEX_INPUT_RATE_VERTEX`：逐顶点读取

#### 顶点属性描述实现

```cpp
std::vector<VkVertexInputAttributeDescription> MyModel::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    return attributeDescriptions;
}
```

- `binding = 0`：从第 0 个顶点缓冲区获取数据
- `location = 0`：对应顶点着色器中 `layout(location = 0) in vec2 position`
- `format = VK_FORMAT_R32G32_SFLOAT`：两个 32 位浮点数，对应 `vec2`
- `offset = 0`：该属性在顶点数据块中的偏移量为 0 (因为 `position` 是第一个属性)

### 创建顶点缓冲区

```cpp
void MyModel::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "At least 3 vertices are required to create a triangle.");

    VkDeviceSize bufferSize = vertexCount * sizeof(Vertex);
    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer,
        vertexBufferMemory);

    void *data;
    vkMapMemory(device.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device.device(), vertexBufferMemory);
}
```

- `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`：标记该缓冲区的用途为顶点缓冲区
- `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`：CPU 可见的内存，允许 CPU 写入
- `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`：一致性内存，CPU 写入后 GPU 自动可见，无需手动刷新缓存
- `vkMapMemory`：将设备内存映射到 CPU 可访问的虚拟地址
- `memcpy`：将顶点数据从 CPU 内存拷贝到映射的设备内存
- `vkUnmapMemory`：解除映射
    - 解除映射后，数据会自动同步到 GPU 内存 (同 `mmap` 释放映射)

#### 直接映射的代价

上述代码在核显或者顶点数据极少的情况下完全没问题。但在有独立显卡的机器上，`HOST_VISIBLE` 的内存实际上是在系统内存里，GPU 需要通过 PCIe 总线去拉取数据

如果每帧都往这里写大量的顶点数据，PCIe 总线会直接变成性能瓶颈

一般采用 Staging Buffer (暂存缓冲区) 来解决这个问题

1. 在 `HOST_VISIBLE | HOST_COHERENT` 的内存中创建一个临时的 Staging Buffer
2. CPU 把数据 `memcpy` 进 Staging Buffer
3. 创建一个 `DEVICE_LOCAL` (纯显存，GPU 访问极快) 的 Vertex Buffer
4. 调用 `copyBuffer` (内存拷贝)，让 GPU 自己把数据从 Staging Buffer 搬到 Vertex Buffer
5. 销毁 Staging Buffer

#### 持久映射 (Persistent Mapping)

`vkMapMemory` 和 `vkUnmapMemory` 在驱动底层是有开销的 (它涉及到虚拟内存到物理内存的页表映射)

如果每帧都要更新顶点数据，绝对不要每帧都 `Map` 和 `Unmap`

正确的做法是在创建 Buffer 时就 `Map` 一次，把返回的 `void* data` 指针存为类的成员变量

之后每次更新数据，直接对着这个指针 `memcpy`，直到程序结束或销毁 Buffer 时，才调用 `Unmap`

### 绑定与绘制

```cpp
void MyModel::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void MyModel::draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}
```

- `offset`: 
- `vkCmdBindVertexBuffers`：将顶点缓冲区绑定到命令缓冲区，参数为 (命令缓冲区, 起始绑定索引, 绑定数量, 缓冲区数组, 偏移量数组)
- `vkCmdDraw`：发出绘制命令，`vertexCount` 从顶点缓冲区中获取，而非硬编码的 `3`

### 加载模型

```cpp
void FirstApp::loadModels()
{
  std::vector<MyModel::Vertex> vertices {
    {{0.0f, -0.5f}},
    {{0.5f, 0.5f}},
    {{-0.5f, 0.5f}},
  };

  model = std::make_unique<MyModel>(device, vertices);
}
```

- 顶点数据从着色器中**搬到了 CPU 端**，由 `MyModel` 负责上传到 GPU
- 构造函数中新增 `loadModels()` 调用，在创建管线之前先加载模型

### 绘制方式变更

之前我们是直接调用 `vkCmdDraw` 硬编码绘制 3 个顶点，顶点数据来自着色器内部数组

但现在由于顶点数据来自 GPU 内存，所以需要先绑定顶点缓冲区，再调用 `vkCmdDraw` 绘制

```cpp
pipeline->bind(commandBuffers[i]);
model->bind(commandBuffers[i]);
model->draw(commandBuffers[i]);
```

### 顶点输入状态补齐

要在渲染管线中配置顶点输入状态，告诉管线如何从顶点缓冲区中读取顶点数据，需要在管线创建时设置

```cpp
auto bindingDescriptions = MyModel::Vertex::getBindingDescriptions();
auto attributeDescriptions = MyModel::Vertex::getAttributeDescriptions();
VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
```
