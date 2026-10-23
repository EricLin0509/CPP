# 片段插值

片段插值 (Fragment Interpolation) 指在光栅化阶段，根据顶点属性通过插值计算出每个片段的属性值

与顶点着色器类似，片段着色器 (Fragment Shader) 中也可使用插值器 (Interpolation) 来接收和计算片段属性数据，通过 `location` 限定符将顶点输出与片段输入一一对应

片段着色器 (Fragment Shader) 在光栅化阶段之后执行，其接收的片段属性值是光栅化阶段通过线性插值 (Linear Interpolation) 计算得出的

## 线性插值

线性插值是在两个已知值之间按比例平滑过渡

一句话理解：**已知两个端点的值，求中间任意一点的值，就按它们距离的比例来分配**

这其实就是我们初中几何里求线段上一点的坐标：`P = A + t(B - A)`

### 为什么需要插值？

GPU是怎么画三角形的：
- 在顶点缓冲区里只给了 **3 个顶点**的颜色 (比如红、绿、蓝)
- 但是经过光栅化后，屏幕上出现了 **几万个像素片段 (fragment)**

那这中间的颜色怎么来？

GPU 不能凭空捏造，只能根据这 3 个顶点的颜色，给内部的所有像素“打分分配”，这个过程就是**插值**

### 屏幕空间里的线性插值

对于三角形内部的一个像素点，GPU 其实是在用**重心坐标 (Barycentric Coordinates)** 算权重

- 如果像素离顶点 A 最近，那它的颜色就最接近 A
- 如果正好在 A 和 B 的正中间，那就是 50% 的 A 加上 50% 的 B

用公式表示 (一维情况)：

- `f(t) = (1 - t) · A + t · B` (其中 `t ∈ [0, 1]`)

**举个例子：**
- 顶点 A 是红色 `(1, 0, 0)`，顶点 B 是蓝色 `(0, 0, 1)`
- 如果 `t = 0.2` (代表该像素在靠近 A 的地方)，那么它的颜色就是：`(1-0.2) * (1,0,0) + 0.2 * (0,0,1) = (0.8, 0, 0.2)`，红中透一点紫

```
A (红色) 
 \ 
  \  (t=0.2) 偏红
   \ 
    * (像素点)
     \
      \  (t=0.8) 偏蓝
       \
        B (蓝色)
```

- GPU 在光栅化阶段对顶点属性（颜色、UV、法线等）做线性插值，为每个片段生成属性值。透视相关属性需做透视校正插值 (Perspective-Correct Interpolation) 避免失真

这样，整个三角形内部就形成了极其平滑的渐变色

但线性插值是一维的，而三角形是二维的，不能直接使用线性插值

那有什么办法在三角型中使用线性差值呢？

这就需要使用**重心坐标 (Barycentric Coordinates)**

## 重心坐标

重心坐标 (Barycentric Coordinates) 相当于线性插值的升级版

它用三角形的三个顶点 A、B、C 按比例混合出内部的任意一点 P (没错，就是比线性插值多出一个同平面的点)

对于三角形内的任意一点 P，都可以通过三个顶点的加权求和来表示: 
`P = u · A + v · B + w · C`

并且这三个权重满足: `u + v + w = 1`
- 其中 `u, v, w` 就是重心坐标

### 几何意义

如果这个点 P 离顶点 A 越近，那么 u 的值就越大，v 和 w 就越小
(在底层，GPU 是通过计算点 P 把三角形划分成的三个小三角形的“面积比例”来求出 `u, v, w` 的值)

如果点 P 正好在三角形的重心上，那么 `u = v = w = 1/3`

### 在 GPU 里怎么用？

只要算出了 `u, v, w`，三角形内任意像素的颜色、UV、法线，都可以用同样的公式插值出来：

```glsl
`PixelColor = u · ColorA + v · ColorB + w · ColorC`
```

这就是 GPU 为什么能用 3 个顶点，平铺出几万个平滑像素的终极秘密

那它是怎么实现如此快的插值运算呢？

#### SIMD Model

GPU 之所以能对几万个片段极速完成插值，核心在于其采用了 **SIMD (Single Instruction Multiple Data)** 架构

- **单指令多数据**：同一条插值指令（如 `f(t) = (1-t)·A + t·B`）同时作用于多个片段的数据
- GPU 的一个 warp/wavefront（通常 32 或 64 个线程）共享同一套指令，只是各自处理不同的片段坐标和权重
- 插值运算本质是乘加（FMA），硬件层面有专用单元，一个时钟周期即可完成

简单来说：CPU 逐个片段算插值，GPU 一批片段同时算——这就是速度差距的根本原因

## 代码实现

### 顶点着色器 (`simple_shader.vert`)

由于新增了颜色属性，顶点着色器需要接收颜色属性

```glsl
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 fragColor;
```

```glsl
void main() {
    ...
    fragColor = color;
}
```

- `layout(location = 1) in vec3 color`：从顶点缓冲区接收颜色属性，`location = 1` 与顶点属性描述中的 `location = 1` 对应
- `layout(location = 0) out vec3 fragColor`：顶点着色器的输出，通过 `location = 0` 传递给片段着色器
- `fragColor = color`：将顶点颜色直接传递出去，GPU 在光栅化阶段会对 `fragColor` 做插值

### 片段着色器 (`simple_shader.frag`)

由于新增了颜色属性，片段着色器需要接收插值后的颜色属性

```glsl
#version 450

layout(location = 0) in vec3 fragColor; // 接收顶点着色器输出的颜色
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

- `layout(location = 0) in vec3 fragColor`：接收顶点着色器输出的插值后颜色，`location = 0` 与顶点着色器的 `out vec3 fragColor` 对应
- 上一章输出固定黄色，本章输出插值后的颜色，三角形内部呈现从红→绿→蓝的平滑渐变

### 顶点结构体变更

由于新增了颜色属性，顶点结构体只需要添加 `glm::vec3 color` 成员即可

```cpp
struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
    
    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};
```

- 新增 `glm::vec3 color` 成员，每个顶点携带颜色数据
- 顶点数据从交错存储 `[Pos][Color][Pos][Color]...` 的方式在同一个缓冲区中排布

### 顶点属性描述变更

对于新增的颜色属性，需要在顶点属性描述中添加 `color` 属性

```cpp
std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
attributeDescriptions[0].binding = 0;
attributeDescriptions[0].location = 0;
attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
attributeDescriptions[0].offset = offsetof(Vertex, position);

attributeDescriptions[1].binding = 0;
attributeDescriptions[1].location = 1;
attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
attributeDescriptions[1].offset = offsetof(Vertex, color);
```

- 属性描述从 1 个增加到 2 个，分别描述 `position` 和 `color`
- `attributeDescriptions[1].location = 1`：对应顶点着色器中 `layout(location = 1) in vec3 color`
- `attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT`：三个 32 位浮点数，对应 `vec3`
- `offsetof(Vertex, color)`：使用 `offsetof` 宏计算 `color` 在 `Vertex` 结构体中的字节偏移量，替代上一章的硬编码 `0`

### 顶点数据变更

只需要每个顶点携带颜色数据，其他数据保持不变

本章：

```cpp
std::vector<MyModel::Vertex> vertices {
  {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
  {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
  {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
};
```

- 每个顶点从只有位置 `{position}` 变为 `{position, color}`
- 三个顶点的颜色分别为红色 `(1, 0, 0)`、绿色 `(0, 1, 0)`、蓝色 `(0, 0, 1)`
- GPU 在光栅化阶段对这三个顶点的颜色做插值，三角形内部呈现 RGB 渐变
