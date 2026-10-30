# 欧拉角 & 齐次坐标

欧拉角 (Euler Angle) 是一种用于描述空间旋转的坐标系统，由三个角度组成，分别表示绕三个轴的旋转角度

齐次坐标 (Homogeneous Coordinates) 是一种用于表示空间点的坐标系统，通过在坐标中添加一个额外的分量来表示点在空间中的位置

## 齐次坐标

“齐次”的意思是说，在数学表达中，这些坐标 (x, y, w) 具有相同的次数 (都是1次)

在之前的[线性转换](../13.2D转换/README.md#线性转换)中，通过一个 2x2 的矩阵来表示空间中的点和线段

但线性转换有个问题：它无法表示平移

因为 2×2 矩阵乘法的结果 `[(a * x) + (b * y)]` 里没有常数项，无论 a、b、c、d 怎么取，都凑不出平移量 tx、ty

其实有种很聪明的解决方案，就是将原来的 2x2 的矩阵

```
[ 缩放, 旋转 ]   [ x ]   [ 新x ]
[ 缩放, 旋转 ] * [ y ] = [ 新y ]

[a, b]   [x]   [(a * x) + (b * y)]
[c, d] x [y] = [(c * x) + (d * y)]
```

添加一个额外的分量并扩展成 3x3 矩阵来计算

```
[ 缩放, 旋转, 平移X ]   [ x ]   [ 新x ]
[ 缩放, 旋转, 平移Y ] * [ y ] = [ 新y ]
[   0,   0,   1   ]   [ 1 ]   [  1  ]

[a, b, tx]   [x]   [(a * x) + (b * y) + tx]
[c, d, ty] x [y] = [(c * x) + (d * y) + ty]
[0, 0,  1]   [1]   [1]
```

这样既**没有改变**原来的缩放和旋转，又可以表示平移

这种坐标转换也叫作2D仿射变换 (2D Affine Transformation)

而通过添加维度的方式实现仿射变换叫作齐次坐标 (Homogeneous Coordinates)

### 应用示例

假设有一个 1×1 的正方形，一个顶点在原点 (0, 0)，对角顶点在 (1, 1)

如果只用线性变换，旋转中心被固定在原点，正方形只能绕 (0, 0) 转

借助齐次坐标，可以让它绕任意一点旋转，例如绕对角顶点 (1, 1)：

1. 把整个正方形平移 (-1, -1)，使 (1, 1) 落到原点
2. 绕原点旋转 θ
3. 把正方形平移回 (1, 1)

写成矩阵连乘（从右往左执行）：

```
[ x' ]   [ 1 0  1 ]   [ cos -sin 0 ]   [ 1 0 -1 ]   [ x ]
[ y' ] = [ 0 1  1 ] · [ sin  cos 0 ] · [ 0 1 -1 ] · [ y ]
[ 1  ]   [ 0 0  1 ]   [  0    0  1 ]   [ 0 0  1 ]   [ 1 ]
```

三个矩阵可以先合成一个：

```
M = [ cos  -sin   1 - cos + sin ]
    [ sin   cos   1 - sin - cos ]
    [  0     0         1        ]
```

代入 (1, 1) 得到 (1, 1)，说明它确实绕 (1, 1) 旋转

一般地，绕任意点 c 旋转的矩阵，平移列就是 c - R·c

注意三步的顺序不能交换，因为矩阵乘法不满足交换律

### 位置与方向：`w` 分量的作用

齐次坐标的第三个分量 `w` 不只是为了凑成 3×3，它还区分了两类东西：

| 类型 | 写法 | 受平移影响 | 受旋转/缩放影响 |
| :--: | :--: | :-----: | :----------: |
| 位置 (Position) | `(x, y, 1)` | 是 | 是 |
| 方向 (Direction) | `(dx, dy, 0)` | 否 | 是 |

方向之所以是 `w = 0`，因为方向本身就是两点之差：

```
(x2, y2, 1) - (x1, y1, 1) = (x2 - x1, y2 - y1, 0)
```

两个点相减，`w` 自动变成 `0`；同时两点的平移量互相抵消：

```
T(c)·P2 - T(c)·P1 = (P2 + c) - (P1 + c) = P2 - P1
```

这说明方向本来就与“在哪”无关

把方向乘上变换矩阵，平移列会被乘成 0 而消失：

```
[ a   b  tx ]   [ dx ]   [ a*dx + b*dy + tx*0 ]   [ a*dx + b*dy ]
[ c   d  ty ] * [ dy ] = [ c*dx + d*dy + ty*0 ] = [ c*dx + d*dy ]
[ 0   0   1 ]   [ 0  ]   [ 0 + 0 + 1*0        ]   [      0      ]
```

对比位置 `(x, y, 1)` 的结果 `a*x + b*y + tx`，唯一的区别就是最后那个乘数：`tx` 乘 `1` 就留下来，乘 `0` 就消失了

所以方向只受线性部分（旋转、缩放）影响，不受平移影响

第三行 `[0, 0, 1]` 还保证了 `w` 不变（`w' = w`），于是点变换后还是点，方向变换后还是方向

用箭头来理解：箭头的含义是“往哪走多远”，跟它画在屏幕哪个位置无关

- 平移整个场景，箭头含义不变
- 旋转整个场景，箭头跟着转
- 缩放整个场景，箭头被拉长压扁（非均匀缩放会让方向变形，这是对的）

这种区分也让齐次坐标的加减法自洽：

| 运算 | `w` 的变化 | 结果 |
| :--: | :--------: | :--: |
| 点 - 点 | `1 - 1 = 0` | 方向 |
| 点 + 方向 | `1 + 0 = 1` | 点 |
| 方向 + 方向 | `0 + 0 = 0` | 方向 |
| 点 + 点 | `1 + 1 = 2` | 无意义 |

总的来说，1 是给点 (位置)， 0 是给向量 (方向)

## 3D变换

现在有个坐标 `(x, y, z)`，通过上述的齐次坐标变换矩阵，它会被转换为齐次坐标 `vec4(pos, 1.0)`

```
[a, b, c, tx]
[d, e, f, ty]
[g, h, i, tz]
[0, 0, 0,  1]
```

- `a, b, c, d, e, f, g, h, i` 是线性变换矩阵 (Linear Transformation)
- `tx, ty, tz` 是平移 (Translation)
- `w = 1` 是位置 (Position)

## 欧拉角

欧拉角 (Euler Angles) 是一种3D空间中的旋转表示方法

它由三个轴 (x, y, z) 组成，分别对应于三种角度的旋转

欧拉角有两套约定，区别在于三次旋转分别绕哪几个轴：

| 类型 | 旋转轴序列 | 特点 | 常见用途 |
| :--: | :------: | :--: | :----: |
| 固有欧拉角 (Proper Euler Angles) | `z-x-z`、`x-y-x`、`y-z-y` | 第一次和第三次绕同一根轴 | 刚体力学：进动、章动、自转 |
| 泰特-布赖恩角 (Tait-Bryan Angles) | `x-y-z`、`z-y-x` | 三个轴各用一次 | 图形学、航空：yaw / pitch / roll |

- 泰特-布赖恩角也叫卡尔丹角 (Cardan Angles) 或航海角 (Nautical Angles)
- 旋转轴序列的顺序也是定义的一部分：`x-y-z` 和 `z-y-x` 是两个不同的约定，结果不同

### 万向节死锁 (Gimbal Lock)

万向节 (Gimbal) 就是陀螺仪里那组嵌套的环，每个环提供一个自由度。欧拉角的三次旋转正好对应三个环

本项目采用 `Y-X-Z` 轴序，中间是 `Rx`。当 `rotation.x = ±90°` 时，外层 Y 轴和内层 Z 轴重合，两个环变成同轴旋转，**丢掉一个自由度**

代入旋转矩阵，此时 `c2 = 0`、`s2 = 1`，结果只剩 `θy - θz` 这个差值，两个角无法区分：

```
R = [ cos(θy-θz)   sin(θy-θz)  0 ]
    [      0            0      -1 ]
    [ -sin(θy-θz)  cos(θy-θz)  0 ]
```

后果是自由度丢失、角度插值时姿态突然翻转、接近 ±90° 时数值不稳定

规避方法是改用四元数或直接累乘旋转矩阵，避免经过角度分解

## 三维顶点

顶点位置由两个分量扩展到三个分量：

```cpp
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};
```

- **`stride = sizeof(Vertex)`**：绑定描述只写总步长，结构体大小变化时这里无需改动
- **`format = VK_FORMAT_R32G32B32_SFLOAT`**：三个 32 位浮点与 `glm::vec3` 一一对应；位置若只按两个分量声明，`color` 的偏移与着色器读取会整体错位
- **`offset = offsetof(Vertex, position)`**：偏移交给编译器计算，避免手写数字与结构体布局脱节

顶点着色器把三维位置补成齐次坐标，再乘变换矩阵：

```glsl
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

void main() {
    gl_Position = push.transform * vec4(position, 1.0);
    fragColor = color;
}
```

- **`vec4(position, 1.0)`**：补上的 `w = 1` 声明它是位置，矩阵的平移列才会生效
- **`gl_Position` 直接接收变换结果**：矩阵最后一行 `[0, 0, 0, 1]` 保证 `w` 仍为 1，裁剪阶段再统一做透视除法
- **颜色走顶点属性**：`fragColor` 由顶点着色器输出，片段着色器写 `vec4(fragColor, 1.0)`，颜色在三角形内部由光栅化插值得到
- 推送常量里的 `color` 字段仍然保留，但着色不再依赖它

## 变换组件

```cpp
struct TransformComponent {
    glm::vec3 translation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation;

    glm::mat4 mat4() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        return glm::mat4{
            {
                scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.0f,
            },
            {translation.x, translation.y, translation.z, 1.0f}};
    }
};
```

- **`rotation` 是弧度制的三个角度**：分别绕 y、x、z 轴
- **编号 1 / 2 / 3 对应的是轴序 (Y, X, Z)，不是分量顺序**：`c1/s1` 取 `rotation.y`，`c2/s2` 取 `rotation.x`，`c3/s3` 取 `rotation.z`

矩阵整体是 `T · R · S`，其中 `R = Ry(θy) · Rx(θx) · Rz(θz)`：

```
Ry = [ c1  0  s1 ]   Rx = [ 1   0   0 ]   Rz = [ c3 -s3  0 ]
     [  0  1   0 ]        [ 0  c2 -s2 ]        [ s3  c3  0 ]
     [-s1  0  c1 ]        [ 0  s2  c2 ]        [  0   0  1 ]
```

三个基本矩阵连乘后：

```
R = [ c1c3 + s1s2s3   -c1s3 + c3s1s2    s1c2 ]
    [ c2s3             c2c3            -s2  ]
    [-s1c3 + c1s2s3     s1s3 + c1c3s2   c1c2 ]
```

这与 `mat4()` 里的数字逐项对应

读代码时需要注意几点：

- **每组花括号是一列**：`glm::mat4` 的构造函数按列填充，所以前三组分别带 `scale.x`、`scale.y`、`scale.z` 的系数，等价于把 `R` 的三列各乘一个缩放因子，即 `S` 在 `R` 的右边
- **第四组是平移列**：`{tx, ty, tz, 1}`，平移直接长在矩阵里，不需要再单独加一个偏移量
- **第四行 `[0, 0, 0, 1]`**：保证 `w` 不变，位置变换后仍然是位置
- **列向量从右往左乘**：先缩放，再 `Rz`，再 `Rx`，最后 `Ry`；缩放写在最右边，所以它沿对象自身的轴发生，非均匀缩放不会跟着旋转方向变形

## 推送常量布局

```cpp
struct SimplePushConstantsData {
  glm::mat4 transform{1.f};
  alignas(16) glm::vec3 color;
};
```

- **`transform` 占 0 ~ 63 字节，`color` 从偏移 64 开始**：整块 80 字节，一次 `vkCmdPushConstants` 就带上变换矩阵与颜色
- **`alignas(16)` 与 std430 对齐规则对齐**：着色器里 `vec3` 按 16 字节对齐，C++ 侧显式声明同样的对齐才能让偏移一致
- **管线布局的 `size = sizeof(SimplePushConstantsData)`**：推送范围的大小必须与着色器声明的块大小相等，偏移 0 覆盖整块

## 旋转动画

```cpp
obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.1f, glm::two_pi<float>());
obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.1f, glm::two_pi<float>());
```

- **每帧各加 0.1 弧度**：两个轴同时转，才能看出 Y-X-Z 轴序叠加的效果；只转一个轴会退化成定轴自转
- **取模用 `two_pi`**：一圈是 2π，转过一整圈后角度回到 0，数值不会随运行时间无限增大（角度越大浮点精度越差）
- **角度存在对象里**：矩阵每帧重新计算，同一个模型能以不同姿态重复绘制

## 立方体模型

```cpp
std::unique_ptr<MyModel> createCubeModel(VulkanDevice& device, glm::vec3 offset) {
  std::vector<MyModel::Vertex> vertices{
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      // ... 其余五个面同理
  };
  for (auto& v : vertices) {
    v.position += offset;
  }
  return std::make_unique<MyModel>(device, vertices);
}
```

- **边长 1、以原点为中心**：六个面各 6 个顶点，共 36 个顶点，每个面由两个三角形拼成
- **顶点展开而不共享**：两个三角形共用一条对角线边，6 个顶点里有 2 个重复；没有索引缓冲时只能用这种写法
- **每个面一种颜色**：没有光照的情况下，若六个面同色，立方体转起来看不出任何变化

| 面 | 颜色 | 值 |
| :--: | :--: | :-- |
| left (左) | 白 | `(0.9, 0.9, 0.9)` |
| right (右) | 黄 | `(0.8, 0.8, 0.1)` |
| top (上) | 橙 | `(0.9, 0.6, 0.1)` |
| bottom (下) | 红 | `(0.8, 0.1, 0.1)` |
| nose (前) | 蓝 | `(0.1, 0.1, 0.8)` |
| tail (后) | 绿 | `(0.1, 0.8, 0.1)` |

- **`offset` 参数整体平移顶点**：复用同一份立方体数据时，不必为每个位置手写绝对坐标

## 场景与坐标约定

```cpp
cube.transform.translation = {.0f, .0f, .5f};  // x: [-1, 1], y: [-1, 1], z: [0, 1]
cube.transform.scale = {.5f, .5f, .5f};
```

- **缩放 0.5 后边长 0.5，中心推到 z = 0.5**：立方体占据 z ∈ [0.25, 0.75]，整个身体落在 NDC 的 [0, 1] 之内
- **深度范围是 0 → 1 而不是 -1 → 1**：`GLM_FORCE_DEPTH_ZERO_TO_ONE` 与 Vulkan 的深度约定一致；深度测试 `VK_COMPARE_OP_LESS` 配清屏值 1.0，z 越小越靠前
- **x、y 的可视范围是 [-1, 1]**：缩放后的立方体只占四分之一宽，绕原点旋转时不会飞出画面
- **Vulkan 的 NDC 里 y 轴向下**：顶点数据中 +y 在屏幕上朝下（立方体注释 `remember y axis points down` 提醒的正是这一点），于是绕 y 轴转是绕竖直轴自转、绕 x 轴转是上下翻转、绕 z 轴转是屏幕内滚动
- **平移、缩放、旋转都在同一个组件里**：三个量最终合成一个 `mat4` 推送，渲染系统只认矩阵，不再关心它由哪几部分组成
