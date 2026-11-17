# 描述符集

在上一章[统一缓冲区](../23.统一缓冲区/README.md)中，我们创建了统一缓冲区，但并没有把它绑定到渲染管线中，所以它在着色器里不会被使用

可是我们没法像顶点缓冲那样，用一条 `vkCmdBindVertexBuffers` 就把缓冲区交给管线，因为 Vulkan 不允许着色器随意访问任意缓冲区：着色器只能在着色器代码里用 `layout(set = N, binding = M)` 声明自己要用哪一份资源，而把真正的缓冲区句柄填到这个编号上的工作，需要另一套机制来完成

这套机制的第一层是描述符 (Descriptor)，它可以理解成 C 语言里一个**带类型的指针**：既保存着资源句柄，也记录着这个句柄该被当成哪种资源来读取

- 缓冲区描述符 (`VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`) 指向一份统一缓冲区的句柄，同时记录了它的偏移与长度
- 图像描述符 (`VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER`) 指向图像视图 (Image View) 与采样器 (Sampler) 的句柄

但描述符本身也不能单独绑定到渲染管线，它必须先被写进描述符集，再以描述符集为单位绑定到管线布局 (Pipeline Layout) 上

描述符集 (Descriptor Set) 就是一组描述符的集合，它把多个资源句柄按 `set`、`binding` 编号排布在一起，绘制时用 `vkCmdBindDescriptorSets` 一次绑定，着色器即可按编号读取到对应资源

这样设计是为了性能，也是它与 OpenGL 资源绑定机制最大的区别：OpenGL 每次绘制都要按名字重新绑定纹理与缓冲区，驱动只能在绘制时临时校验这些状态；Vulkan 则把绑定关系提前固化进描述符集与管线布局，绘制时只剩一次绑定调用，校验也提前到了创建阶段

## 场景示例

假设场景里同时要用到这几份资源：

| 描述符 | 类型 | 内容 | 更新频率 |
| :---: | :---: | :---: | :---: |
| 1 | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | 全局统一缓冲区：视图/投影矩阵、光照方向 | 每帧一次，所有物体共享 |
| 2 | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | 材质统一缓冲区：反照率、粗糙度 | 每个物体各一份 |
| 3 | `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | 反照率纹理 | 每个物体各一份 |
| 4 | `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | 法线纹理 | 每个物体各一份 |

这四个描述符不必塞进同一个描述符集，可以按**更新频率**分成两个：

- set 0 只放描述符 1，每帧更新一次，所有物体共用
- set 1 放描述符 2、3、4，每个物体一份，绘制不同物体时才需要更换

着色器里通过 `set` 与 `binding` 两级编号定位到它们：set 0 的 `binding = 0` 声明全局统一缓冲区块，set 1 的 `binding = 0`、`binding = 1`、`binding = 2` 则分别声明材质统一缓冲区块与两张 `sampler2D` 纹理

- **每个描述符集的 `binding` 编号各自从 0 开始**：set 0 的 `binding = 0` 与 set 1 的 `binding = 0` 是两个互不相关的描述符
- **同一个 `binding` 编号在同一个 set 内只能对应一份资源**：这也是描述符集布局要在创建管线布局时就确定下来的原因

绘制时的绑定顺序则完全对应这个划分：进入绘制循环之前，先用 `vkCmdBindDescriptorSets` 把 set 0 绑一次，此后循环内每切换一个物体，再绑定该物体自己的 set 1

- **起始编号与数量是两个独立参数**：`firstSet` 指明从第几个 set 开始绑，`descriptorSetCount` 指明连续绑几个，`firstSet = 0`、`descriptorSetCount = 1` 就只绑 set 0
- **只重绑变化的那一个 set**：set 0 的内容在整帧里不变，重复绑定只会白白增加开销，这正是按频率拆分描述符集的目的
- **也可以一次绑定连续的多个 set**：`firstSet = 0`、`descriptorSetCount = 2`，但这样每次绘制都会把不变的 set 0 也一起重绑
- **管线布局必须按顺序声明所有用到的 set**：`pSetLayouts` 的数组下标就是 `set` 编号，少声明一个，着色器里对应的 `set` 就取不到资源
- **`pipelineLayoutInfo.setLayoutCount` 是 1**：`pSetLayouts` 里只有全局 set 0 的布局，材质 set 1 还没有出现

## 描述符集布局

描述符集布局 (Descriptor Set Layout) 只描述**一个描述符集里长什么样**，不涉及任何真实资源

它规定了有几份描述符、每份的 `binding` 编号是多少、属于哪种类型、需要几个、在哪些着色器阶段可见

单个描述符用 `VkDescriptorSetLayoutBinding` 描述，它的五个字段分别对应上面这些信息：

- **`binding` 必须与着色器里的编号一致**：`layout(set = 0, binding = 0)` 对应 `binding = 0`，对不上就取不到资源
- **`descriptorType` 必须与描述符实际类型一致**：统一缓冲区写 `UNIFORM_BUFFER`，纹理写 `COMBINED_IMAGE_SAMPLER`，类型不匹配属于未定义行为
- **`descriptorCount` 是数组长度**：普通单个资源填 1，着色器里写成 `sampler2D maps[4]` 时填 4
- **`stageFlags` 声明在哪些阶段可见**：只在顶点着色器用就写 `VERTEX_BIT`，两个阶段都用就按位或起来
- **`pImmutableSamplers` 通常为空**：只有采样器参数在创建布局时就固定下来才需要填

把若干 `VkDescriptorSetLayoutBinding` 通过 `bindingCount` 与 `pBindings` 交给 `VkDescriptorSetLayoutCreateInfo`，再调用 `vkCreateDescriptorSetLayout`，就得到一个描述符集布局

它和管线布局的关系是**层层套嵌**：

- 管线布局 (Pipeline Layout) 是最外层的容器
  - set 0 → 一个描述符集布局，内部有 `binding` 0、1、2 ……
  - set 1 → 另一个描述符集布局，内部的 `binding` 同样从 0 开始

- **一个描述符集布局描述一个 set**：上一节的 set 0 与 set 1 各自需要一个布局对象
- **管线布局通过 `pSetLayouts` 把它们按 `set` 编号顺序串起来**：数组下标就是 `set` 的值，第一个元素对应 set 0
- **布局与描述符集是"模板与实例"的关系**：布局只在创建时用一次，描述符集则是按这个模板分配出来、再往里填真实句柄的对象
- **布局可以复用**：多个管线如果描述符结构相同，可以共用同一个 `VkDescriptorSetLayout`，不必重复创建
- **创建与销毁对称**：`vkCreateDescriptorSetLayout` 创建，`vkDestroyDescriptorSetLayout` 释放，并且要在用到它的管线布局销毁之后再销毁

### 描述符集布局封装

`MyDescriptorSetLayout` 把这一节的内容封装成一个类，`binding` 的五个字段在构建时填入：

```cpp
class MyDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(VulkanDevice &device) : device{device} {}

    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<MyDescriptorSetLayout> build() const;

   private:
    VulkanDevice &device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };
  ...
};
```

```cpp
MyDescriptorSetLayout::Builder &MyDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings[binding] = layoutBinding;
  return *this;
}
```

- **`addBinding` 返回自身引用**：可以连续调用，把多个 `binding` 串成一条表达式
- **重复的 `binding` 编号直接断言失败**：`bindings.count(binding) == 0` 保证同一个编号不会写两次
- **`count` 默认 1**：单个资源不必显式传参
- **`pImmutableSamplers` 由 `{}` 零初始化**：聚合初始化把整个结构清零，这个字段因此是 `nullptr`
- **`bindings` 用 `unordered_map` 而不是 `vector`**：按编号查找，`MyDescriptorWriter` 写入时要靠编号取出类型与数量
- **`build()` 把这张表按值传进构造函数**：`bindings` 的副本同时被布局对象保存下来，供后续写入查询

构造函数把这张表转成 Vulkan 需要的数组，再创建布局对象：

```cpp
MyDescriptorSetLayout::MyDescriptorSetLayout(
    VulkanDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : device{device}, bindings{bindings} {
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(
          device.device(),
          &descriptorSetLayoutInfo,
          nullptr,
          &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

MyDescriptorSetLayout::~MyDescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
}
```

- **遍历顺序不影响结果**：`unordered_map` 的迭代顺序不确定，但每个元素的 `binding` 字段都是显式写好的编号，`vkCreateDescriptorSetLayout` 按字段而不是按数组下标匹配
- **`pBindings` 指向局部 `vector` 的数据**：`vkCreateDescriptorSetLayout` 在调用返回前就把内容拷走，函数结束后这个 `vector` 析构也没问题
- **失败即抛异常**：`std::runtime_error` 会让构造中断，不会留下半构造的对象
- **析构函数对应销毁**：`vkDestroyDescriptorSetLayout` 与构造里的创建一一配对
- **拷贝构造与拷贝赋值被删除**：布局句柄唯一，复制会造成重复销毁
- **`bindings` 成员在创建后仍然保留**：它不参与运行时绑定，只给 `MyDescriptorWriter` 查询每个 `binding` 的类型与数量
- **`friend class MyDescriptorWriter` 声明在类尾**：`bindings` 是私有的，写入器需要直接读取它，因此必须被声明为友元

## 描述符池

我们无法直接创建描述符集，必须先创建一个描述符池 (Descriptor Pool)

这也是为了性能考虑，因为一次内存分配是非常昂贵的

描述符池的思路和对象池 (Object Pool) 一样：与其为每一个描述符集单独向驱动申请一次内存，不如先一次性申请一大块，之后所有的描述符集都从这块内存里切出来

这样一来，分配一个描述符集就退化成"从池里挑一块空闲槽位"，不需要真正走内存分配，代价小得多，也避免了频繁分配导致的内存碎片

创建池时只需要回答两个问题：

- **能装多少个描述符集**：这个上限决定了池最多能分配出几个 set
- **每种描述符各能装多少个**：按类型分别给出总量，例如统一缓冲区描述符多少个、纹理描述符多少个

有两个容易误解的地方：

- **数量是按类型算的整池总量，不是每个描述符集的数量**：池里所有 set 加起来能用的统一缓冲区描述符不能超过这个数
- **两个上限互相独立**：一个限制集的个数，一个限制每种描述符的个数，哪一个先被用尽都会导致分配失败

容量不足时分配会直接失败，池里的槽位被用得很零碎时也可能失败

描述符池只提供内存，不管形状：池本身不知道一个 set 里有几份描述符、分别是什么类型，这些信息都来自描述符集布局，分配时由布局告诉池该扣掉哪些类型、各扣多少

按前面场景示例的划分，可以这样估算一个池的容量：

- 需要分配的集：全局 set 0 每个飞行帧一份，材质 set 1 每个物体一份
- 假设 2 个飞行帧、N 个物体，则集的数量是 2 + N
- 统一缓冲区描述符的总数是 2 + N（全局 UBO 与材质 UBO 各占一份）
- 纹理描述符的总数是 2N（每个物体的反照率与法线纹理各一份）

描述符集的生命周期完全交给池管理，这一点和缓冲区不同：

- **销毁池就等于销毁池里所有描述符集**：不需要逐个释放
- **整池重置**：一次操作让池里所有描述符集同时失效，内存重新变为空闲，可以再次分配
- **单独释放要额外声明**：默认只能整池回收，创建池时特意开启相应选项，才允许逐个释放描述符集
- **重置有前提**：命令缓冲里还绑着这个池的集时不能重置，否则录制好的命令会引用到已失效的集，通常的做法是每个飞行帧一个池，或者等对应的栅栏 (Fence) 确认这一帧真的执行完

从整体流程看，前面几节加上这一节构成一条完整的链：

- 描述符集布局决定一个 set 的形状
- 描述符池提供承载这些 set 的内存
- 按布局从池里分配出描述符集
- 把真实资源句柄写进描述符集
- 绘制时把描述符集绑定到管线布局上

销毁顺序和创建顺序相反：

- 先销毁管线布局，因为它引用了各个描述符集布局
- 再销毁描述符池，它负责销毁池里的描述符集
- 最后销毁各个描述符集布局，此时已经没有任何对象在引用它们

### 描述符池封装

`MyDescriptorPool` 把池的两个上限与分配动作封装起来，构建器带默认值：

```cpp
class MyDescriptorPool {
 public:
  class Builder {
   public:
    Builder(VulkanDevice &device) : device{device} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<MyDescriptorPool> build() const;

   private:
    VulkanDevice &device;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };
  ...
};
```

- **`maxSets` 默认 1000**：不调用 `setMaxSets` 时池最多容纳 1000 个描述符集
- **`poolFlags` 默认 0**：没有开启 `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT`，池只能整池回收
- **`poolSizes` 是 `vector` 而不是 `map`**：同一类型不会重复累加，多次 `addPoolSize` 只是追加条目
- **`addPoolSize` 用聚合初始化 `{descriptorType, count}`**：直接对应 `VkDescriptorPoolSize` 的两个字段

```cpp
MyDescriptorPool::Builder &MyDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}
```

池对象的构造与分配：

```cpp
MyDescriptorPool::MyDescriptorPool(
    VulkanDevice &device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : device{device} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

bool MyDescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}
```

- **`allocateDescriptor` 每次只分配一个 set**：`descriptorSetCount = 1`，`pSetLayouts` 指向单个布局句柄
- **失败返回 `false` 而不是抛异常**：容量不足是预期内的结果，调用方可以据此决定是否重建池
- **`allocateDescriptor` 是 `const` 成员**：分配动作修改的是池的内部状态，但 `descriptorPool` 句柄本身不变，方法因此可以声明为 `const`
- **参数是裸句柄 `VkDescriptorSetLayout`**：布局对象的所有权仍在 `MyDescriptorSetLayout`，池只借用它的句柄
- **`freeDescriptors` 与 `resetPool` 分别对应逐个释放与整池重置**：前者需要池创建时开启释放选项，后者不受这个限制

## 描述符写入器

`MyDescriptorWriter` 负责"分配 + 写入"这两步，它同时引用布局与池：

```cpp
class MyDescriptorWriter {
 public:
  MyDescriptorWriter(MyDescriptorSetLayout &setLayout, MyDescriptorPool &pool);

  MyDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  MyDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

 private:
  MyDescriptorSetLayout &setLayout;
  MyDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
```

```cpp
MyDescriptorWriter &MyDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  writes.push_back(write);
  return *this;
}
```

- **`descriptorType` 取自布局而不是参数**：调用方不必再写一遍类型，也就不可能与布局里的声明不一致
- **两条断言各自把关**：编号必须在布局里存在，且该编号的 `descriptorCount` 必须是 1，因为这个接口一次只写一份资源
- **`dstBinding` 就是传入的编号**：`dstSet` 此时留空，等 `overwrite` 时才填
- **`pBufferInfo` 保存的是指针**：`VkDescriptorBufferInfo` 必须在 `vkUpdateDescriptorSets` 调用时仍然有效
- **`writes` 按值保存 `VkWriteDescriptorSet`**：一次写入多个 `binding` 时，它们被累积起来，最后统一提交
- **`writeImage` 与 `writeBuffer` 结构相同**：只把 `pBufferInfo` 换成 `pImageInfo`

分配与写入的衔接：

```cpp
bool MyDescriptorWriter::build(VkDescriptorSet &set) {
  bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void MyDescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool.device.device(), writes.size(), writes.data(), 0, nullptr);
}
```

- **`build` 先分配再写入**：分配失败就直接返回 `false`，不进入写入阶段
- **`overwrite` 先给每条写入填上 `dstSet`**：同一个写入器积累的若干 `binding` 都指向同一个描述符集
- **`vkUpdateDescriptorSets` 的后两个参数是 0 与 `nullptr`**：不做拷贝，只做写入
- **`overwrite` 是公开方法**：已经分配好的描述符集可以不重新分配，直接覆盖里面的内容
- **`pool.device` 被直接访问**：`device` 是 `MyDescriptorPool` 的私有成员，`MyDescriptorPool` 里声明了 `friend class MyDescriptorWriter` 才允许这样用

## 全局描述符集的创建与写入

`FirstApp` 的构造函数先建池：

```cpp
FirstApp::FirstApp() { 
  globalPool = MyDescriptorPool::Builder(device)
    .setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
    .build();

  loadGameObjects();
}
```

- **`maxSets` 与 UBO 描述符数量都是 `MAX_FRAMES_IN_FLIGHT`**：两个飞行帧各要一个 set，每个 set 里各有一份统一缓冲区描述符
- **池在 `loadGameObjects()` 之前创建**：池是成员，模型加载时它已经就绪
- **`Builder(device)` 是临时对象**：`.setMaxSets(...)` 与 `.addPoolSize(...)` 都返回 `Builder&`，链式调用结束后由 `.build()` 产出 `unique_ptr`
- **`poolFlags` 保持默认的 0**：这个池不支持逐个释放描述符集

`run()` 里按"布局 → 池 → 集 → 写入"的顺序完成这几步：

```cpp
auto globalSetLayout = MyDescriptorSetLayout::Builder(device)
  .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
  .build();

std::vector<VkDescriptorSet> globalDescriptorSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
for (int i = 0; i < globalDescriptorSets.size(); i++) {
  auto bufferInfo = uboBuffers[i]->descriptorInfo();
  MyDescriptorWriter(*globalSetLayout, *globalPool)
    .writeBuffer(0, &bufferInfo)
    .build(globalDescriptorSets[i]);
 }
```

- **只声明一个 `binding`**：`binding = 0` 的类型是 `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`，`count` 取默认值 1
- **`stageFlags` 只写 `VK_SHADER_STAGE_VERTEX_BIT`**：全局 UBO 只在顶点着色器里读取
- **布局与 UBO 缓冲区都是 `run()` 的局部对象**：布局是 `unique_ptr`，`uboBuffers` 是 `vector<unique_ptr<MyBuffer>>`，两者随 `run()` 结束一起释放
- **每个飞行帧一个描述符集**：`globalDescriptorSets` 的长度同样是 `MAX_FRAMES_IN_FLIGHT`，下标与 `frameIndex` 对应
- **`descriptorInfo()` 不带参数**：`size` 取默认的 `VK_WHOLE_SIZE`、`offset` 取 0，描述符范围覆盖整个 76 字节缓冲区
- **`bufferInfo` 是循环内的局部变量**：它的地址被写进 `VkWriteDescriptorSet`，而 `.build()` 在同一次迭代里就调用 `vkUpdateDescriptorSets`，指针在整个使用期间都有效
- **`MyDescriptorWriter(...)` 也是临时对象**：`writeBuffer` 返回 `*this`，`.build()` 在同一个表达式里完成分配与写入
- **写入器与描述符集一一对应**：每个飞行帧的 set 各自写入自己那份 UBO

## 管线布局接入 set 0

渲染系统的构造函数与 `createPipelineLayout` 都接收一个布局句柄：

```cpp
SimpleRenderSystem::SimpleRenderSystem(
    VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : device(device) {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}
```

```cpp
void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantsData);

  std::vector<VkDescriptorSetLayout> setLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
  pipelineLayoutInfo.pSetLayouts = setLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}
```

- **`setLayouts` 是单元素 `vector`**：下标 0 就是 set 0，正好对应着色器里的 `layout(set = 0, ...)`
- **`setLayoutCount` 由 `setLayouts.size()` 算出**：数量与数组长度始终一致
- **`pSetLayouts` 指向局部 `vector` 的数据**：`vkCreatePipelineLayout` 在返回前拷走内容，`vector` 随后析构不影响结果
- **推送常量范围不变**：仍是 128 字节、偏移 0、两个阶段可见
- **布局句柄是借用而非持有**：`VkDescriptorSetLayout` 的所有权在 `MyDescriptorSetLayout`，管线布局只引用它，所以布局对象必须比管线布局活得更久
- **调用点在 `my_app.cpp` 的 `run()`**：`SimpleRenderSystem renderSystem{device, renderer.getRenderPass(), globalSetLayout->getDescriptorSetLayout()};`，传进去的是裸句柄

## 着色器里的声明

顶点着色器用 `set` 与 `binding` 声明全局统一缓冲区块：

```glsl
layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec3 directionToLight;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;
```

- **`set = 0` 对应管线布局里 `pSetLayouts[0]`**：两级编号必须与布局对象完全对上
- **`binding = 0` 对应 `addBinding(0, ...)`**：编号相同才能取到那份资源
- **字段名与 C++ 侧不同**：C++ 结构里叫 `ProjectionMatrix` 与 `lightDirection`，着色器里叫 `projectionViewMatrix` 与 `directionToLight`；两边只按偏移与类型对应，名字不参与匹配
- **`ubo` 是块实例名**：访问成员要写成 `ubo.projectionViewMatrix`，与推送常量块的 `push.` 前缀同理
- **`vec3` 在 std140 里占 16 字节**：偏移 64 处对齐，块的总长度是 80 字节，而描述符范围是 76 字节，覆盖了实际用到的 `[0, 76)`
- **块内容与 C++ 的 `GlobalUbo` 逐字段对应**：一个 `mat4` 加一个 `vec3`，顺序也一致

顶点的裁剪坐标由 UBO 与推送常量共同算出：

```glsl
gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);
```

- **`P * V` 来自 UBO**：所有物体共用一份，每帧只更新一次
- **`M` 来自推送常量**：每个物体各不相同，仍在循环里逐个推送
- **两次矩阵乘都在顶点着色器里执行**：CPU 只分别提供 `P * V` 与 `M`，不预先相乘
- **顺序不能交换**：先 `M` 把顶点送入世界空间，再 `P * V` 送入裁剪空间

推送常量块的两个字段：

```cpp
struct SimplePushConstantsData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};
```

```cpp
SimplePushConstantsData push = {};
push.modelMatrix = obj.transform.mat4();
push.normalMatrix = obj.transform.normalMatrix();
```

- **第一块是模型矩阵**：与 GLSL 里 `push.modelMatrix` 的位置一致，偏移 0
- **第二块是法线矩阵**：偏移 64，GLSL 里对应 `push.normalMatrix`
- **`normalMatrix()` 返回的是 `glm::mat3`**：赋给 `glm::mat4` 成员时保留左上角 3×3，第四行与第四列取单位阵的值，实测对角末尾为 1、其余为 0
- **法线矩阵在 CPU 上算好**：着色器里 `mat3(push.normalMatrix)` 直接取到 `R * S⁻¹`，不需要在着色器里求逆

光照方向从 UBO 读取：

```glsl
const float AMBIENT = 0.02;

float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0.0);
```

- **方向只由 `ubo.directionToLight` 提供**：着色器里没有方向常量，改方向只需改 CPU 侧的 `GlobalUbo`
- **`AMBIENT` 仍是着色器常量**：它没有进 UBO，改它要重新编译着色器
- **`ubo.directionToLight` 已是单位向量**：C++ 侧用 `normalize` 初始化，所以点积直接就是余弦

片元着色器只声明推送常量块，不声明 set 0：

```glsl
layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

- **两个阶段的推送常量块必须同名同布局**：`modelMatrix` 与 `normalMatrix` 的类型与顺序都一致
- **片元阶段不读块内容**：块声明只是为了让两个阶段共用同一份推送常量范围
- **`binding = 0` 的 `stageFlags` 只有 `VERTEX_BIT`**：片元着色器没有声明 set 0，访问该资源在片元阶段本来也不可见

## 每帧绑定描述符集

绘制前先绑定管线，再绑定描述符集：

```cpp
void SimpleRenderSystem::renderGameObjects(
    FrameInfo& frameInfo,
    std::vector<GameObject>& gameObjects)
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

  for (auto& obj : gameObjects) {
```

- **`VK_PIPELINE_BIND_POINT_GRAPHICS`**：图形管线绑定的描述符集，与 `vkCmdBindPipeline` 绑的是同一种绑定点
- **`firstSet = 0`、`descriptorSetCount = 1`**：只绑 set 0 这一个
- **`&frameInfo.globalDescriptorSet`**：长度必须不少于 `descriptorSetCount`，这里只有一个元素
- **`dynamicOffsetCount = 0` 与 `nullptr`**：没有使用动态偏移，描述符的偏移已经在 `VkDescriptorBufferInfo` 里写死
- **绑定发生在循环之外**：所有物体共用同一个 set 0，循环里只换推送常量与顶点缓冲
- **顺序是管线在前、描述符集在后**：`vkCmdBindDescriptorSets` 里的 `pipelineLayout` 必须与当前绑定的管线兼容
- **`pipelineLayout` 是渲染系统的成员**：绑定描述符集与推送常量都用它
- **循环里的推送常量按物体逐个调用**：`vkCmdPushConstants` 与每帧一次的描述符集绑定形成两级更新频率

## 帧信息

`FrameInfo` 带一个描述符集句柄：

```cpp
struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    MyCamera camera;
    VkDescriptorSet globalDescriptorSet;
};
```

- **`VkDescriptorSet` 是值类型句柄**：直接按值存放，不需要智能指针
- **它是第五个字段**：聚合初始化时排在 `camera` 之后
- **构造时填入本帧的那一份**：`FrameInfo frameInfo{frameIndex, deltaTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};`
- **下标与 `frameIndex` 一致**：描述符集、UBO 缓冲区、命令缓冲区三者按同一个帧索引配对
- **渲染系统只读它**：`vkCmdBindDescriptorSets` 需要它的地址

`my_app.hpp` 里池作为成员持有：

```cpp
  // order of declarations matters
  std::unique_ptr<MyDescriptorPool> globalPool{};
  std::vector<GameObject> gameObjects;
```

- **池是成员而不是局部变量**：它要跨越 `run()` 的整个生命周期
- **`std::unique_ptr` 声明即初始化**：`{}` 让它一开始是空指针，构造函数里再赋值
- **成员按声明顺序的逆序析构**：`gameObjects` 先于 `globalPool` 销毁，注释 `order of declarations matters` 提示这个顺序是被刻意安排的
- **头文件需要包含 `my_descriptors.hpp`**：`MyDescriptorPool` 是不完整类型时无法构造 `unique_ptr` 的析构器
