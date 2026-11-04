# 透视矩阵

此次我们将讲解[透视矩阵](../16.透视投影/README.md)的具体实现

## 相机对象

相机 (Camera) 目前只持有一个投影矩阵：

```cpp
class MyCamera {
    public:
        void setOrthographicProjection(
            float left, float right, float top, float bottom, float near, float far);

        void setPerspectiveProjection(float fovy, float aspect, float near, float far);

        const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

    private:
        glm::mat4 projectionMatrix{1.f};
};
```

- **只存矩阵，不存位置和朝向**：相机暂时固定在原点、朝 `+z` 看，所以没有视图矩阵 (View Matrix)；`glm::mat4{1.f}` 是单位阵，未设置投影时相当于什么都不做
- **两个 `set` 函数各写一个矩阵**：正交与透视互斥，后调用的会覆盖前一个
- **`getProjectionMatrix()` 返回 `const` 引用**：矩阵有 64 字节，没必要按值返回；返回引用也意味着调用方只能读不能改
- **参数全部是相机空间 (Camera Space) 的边界**：`left/right/top/bottom` 是近裁剪面上的矩形范围，`near/far` 是两张裁剪面到相机的距离

## 正交投影

```cpp
void MyCamera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far)
{
  projectionMatrix = glm::mat4{1.0f};
  projectionMatrix[0][0] = 2.f / (right - left);
  projectionMatrix[1][1] = 2.f / (bottom - top);
  projectionMatrix[2][2] = 1.f / (far - near);
  projectionMatrix[3][0] = -(right + left) / (right - left);
  projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
  projectionMatrix[3][2] = -near / (far - near);
}
```

这就是[16.透视投影](../16.透视投影/README.md#正交投影)里那个 `平移 + 缩放` 的矩阵，只是把它拆成逐元素的赋值：

- **先置为单位阵**：`glm::mat4{1.0f}` 让 `[3][3] = 1`、其余位置为 0，之后只改六个元素，剩余位置自动保持"不参与计算"的值
- **`[0][0]` 与 `[3][0]` 是 x 行的缩放和平移**：`2/(r-l)` 把宽度 `r-l` 拉成 NDC 的宽度 2，`-(r+l)/(r-l)` 把区间中心 `(r+l)/2` 挪到 0
- **`[2][2]` 与 `[3][2]` 是 z 行**：`1/(f-n)` 而不是 `2/(f-n)`，因为深度区间是 `[0, 1]` 而不是 `[-1, 1]`；偏移 `-n/(f-n)` 把 `z = n` 映到 0
- **`[1][1]` 与 `[3][1]` 用了 `(bottom - top)` 而不是 `(top - bottom)`**：y 行比教科书多一个负号，这就是 Vulkan y 轴向下（NDC 顶部是 `y = -1`）的适配

### y 行与深度范围

y 行这个负号的效果取决于传入的参数顺序：

| `top`, `bottom` | `[1][1]` | `[3][1]` | 效果 |
| :--: | :--: | :--: | :-- |
| `1`, `-1` (数学惯例，y 向上) | `2/(-2) = -1` | `-(0)/(-2) = 0` | y 被翻转 |
| `-1`, `1` (Vulkan 惯例，y 向下) | `2/(2) = 1` | `-(0)/(2) = 0` | y 原样通过 |

调用处用的是第二种：

```cpp
// camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
```

四个区间参数代入后：

- **x**：`[-aspect, aspect]` → `[-1, 1]`，缩放 `1/aspect`
- **y**：`[-1, 1]` → `[-1, 1]`，缩放 `1`、无偏移，即不做任何改变
- **z**：`[-1, 1]` → `[0, 1]`，缩放 `0.5`、偏移 `0.5`

顶点数据里 +y 本来就代表"屏幕下方"，所以 y 不需要再翻转；而 z 从 `[-1, 1]` 变成 `[0, 1]` 是 Vulkan 深度范围要求的，与 `GLM_FORCE_DEPTH_ZERO_TO_ONE` 一致

## 透视投影

```cpp
void MyCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
{
  assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tanHalfFovy = tan(fovy / 2.f);
  projectionMatrix = glm::mat4{0.0f};
  projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
  projectionMatrix[1][1] = 1.f / (tanHalfFovy);
  projectionMatrix[2][2] = far / (far - near);
  projectionMatrix[2][3] = 1.f;
  projectionMatrix[3][2] = -(far * near) / (far - near);
}
```

- **`assert` 针对的是"宽高比恰好等于浮点误差"这种退化情况**：判断式是 `abs(aspect - epsilon) > 0`，只有 `aspect` 与 `epsilon` 相等时才会触发；真正的风险在后面的 `1/(aspect * tanHalfFovy)`——宽高比为 0 时它会除零，所以调用前必须保证交换链尺寸有效
- **`tanHalfFovy = tan(fovy / 2)`**：`fovy` 是垂直方向的整个张角，而视锥的上下边界关于光轴对称，用的是半角
- **`fovy` 是弧度**：`GLM_FORCE_RADIANS` 让 glm 的三角函数按弧度计算，所以调用处先 `glm::radians(50.f)` 把角度转过来
- **`glm::mat4{0.0f}` 全零初始化**：`[3][3] = 0` 是关键——齐次坐标的 `w` 不再保持原值，而要由 z 提供

### 从视锥到矩阵

这里没有显式给出 `l/r/t/b`，而是假设视锥在水平和垂直方向都关于光轴对称，只由 `fovy` 与 `aspect` 推出边界：

```
t = n * tanHalfFovy
b = -t
r = aspect * t
l = -r
```

代入[16.透视投影](../16.透视投影/README.md#透视投影变换)的通用矩阵，对称化后中间项全部消掉：

```
2n / (r - l) = 2n / (2 * aspect * n * tanHalfFovy) = 1 / (aspect * tanHalfFovy)
2n / (t - b) = 2n / (2 * n * tanHalfFovy)         = 1 / tanHalfFovy
-(r + l) / (r - l) = 0
-(t + b) / (t - b) = 0
```

所以前两行只剩下对角线，这也解释了代码里没有出现 `l/r/t/b` 的原因

z 行照抄通用形式：`f/(f-n)` 与 `-fn/(f-n)`，其中 `-fn/(f-n)` 就是[为什么是非线性](../16.透视投影/README.md#为什么是非线性)里那个常数项 `-fn/(f-n)`

注意 `glm::mat4` 按列存储，`projectionMatrix[列][行]` 的下标顺序是反的：

- **`[2][3] = 1`**：第 3 行第 2 列，让 `w' = z`，透视除法的除数是深度
- **`[3][2] = -(fn)/(f-n)`**：第 2 行第 3 列，深度的平移项

最终矩阵：

```
[ 1/(aspect * tanHalfFovy)      0                0               0     ]
[            0          1/tanHalfFovy           0                0     ]
[            0                  0          f/(f-n)        -fn/(f-n)    ]
[            0                  0                1               0     ]
```

### `w` 与后续阶段

顶点着色器里 `gl_Position` 拿到的是裁剪空间坐标 (Clip Space)：

```
gl_Position = push.transform * vec4(position, 1.0);
```

- **`w` 不再等于 1**：矩阵第 3 行让 `w = z`，`gl_Position.xyz` 里装的是还没除过的 `nx`、`ny`、`(f/(f-n))z - fn/(f-n)`
- **透视除法不在着色器里做**：`ndc = clip.xyz / clip.w` 由固定功能阶段在裁剪与视口变换时完成，着色器代码一行都不用改
- **深度变成非线性**：`z_ndc` 是 `(f/(f-n))z - fn/(f-n)` 再除以 `z`，这就是近处精度高、远处精度低的那条曲线

## 宽高比

宽高比来自交换链，经由渲染器转发：

```cpp
// my_engine_swap_chain.hpp
float extentAspectRatio() {
  return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
}

// my_renderer.hpp
float getAspectRatio() const { return swapchain->extentAspectRatio(); }
```

- **宽度除以高度**：`800 x 600` 得到 `1.333...`，即 4:3
- **交换链归渲染器所有，所以接口放在渲染器上**：应用层不需要直接接触 `VulkanSwapChain`
- **只用 `width` 和 `height`，不用 `minDepth` 之类**：投影只关心画面的形状，与像素格式无关

宽高比之所以必须参与投影，是因为 NDC 是固定的一比一正方形：

- **`x` 和 `y` 的缩放不相等**：`[0][0]` 比 `[1][1]` 多除了一个 `aspect`
- **否则画面被拉伸**：若两个方向都用 `1/tanHalfFovy`，4:3 的窗口会把内容压成正方形，圆变成椭圆，立方体变成长方体
- **`aspect` 每帧重新计算**：交换链重建后尺寸会变，投影矩阵跟着更新，窗口缩放时物体不会变形

## 投影与模型矩阵相乘

渲染系统的签名多了一个相机参数，推送常量里放的是两者的乘积：

```cpp
void SimpleRenderSystem::renderGameObjects(
    VkCommandBuffer commandBuffer,
    std::vector<GameObject>& gameObjects,
    MyCamera& camera)
{
  pipeline->bind(commandBuffer);

  for (auto& obj : gameObjects) {
    // ... 累加旋转角 ...
    SimplePushConstantsData push = {};
    push.color = obj.color;
    push.transform = camera.getProjectionMatrix() * obj.transform.mat4();
    // ... vkCmdPushConstants ...
  }
}
```

- **`投影 * 模型`，顺序不能反**：顶点先做模型变换进入世界空间，再做投影进入裁剪空间；矩阵从右往左作用，所以投影写在左边
- **投影对所有对象相同**：它描述的是"相机怎么看待世界"，与物体自身的位置姿态无关；因此它是循环外唯一不变的那部分
- **模型矩阵仍由 `TransformComponent` 提供**：`mat4()` 只管平移、旋转、缩放，投影的知识完全不进入变换组件
- **推送常量的大小不变**：仍然是 `mat4 + vec3`，一个矩阵同时承担了两种变换，`vkCmdPushConstants` 的字节数无需调整
- **渲染系统不持有相机**：相机以引用传入，渲染系统只读它，投影矩阵的更新节奏由应用层控制

### 为什么相机是"外置"的

```
应用层 ──> MyCamera（投影矩阵）
              │
              └──> SimpleRenderSystem.renderGameObjects(camera)
                        │
                        └──> push.transform = P * M
```

- **渲染系统只认识"一个矩阵"**：它不知道视场角、宽高比、近远平面，换一种投影（正交、透视）不需要改渲染系统
- **相机不进入 `GameObject`**：相机不是被渲染的物体，它的参数属于整个场景而不是某个对象
- **后续加视图矩阵时只需改乘法顺序**：届时是 `P * V * M`，仍然在同一个推送常量里完成

## 场景摆放

```cpp
cube.transform.translation = {.0f, .0f, 2.5f};
cube.transform.scale = {.5f, .5f, .5f};
```

配合 `setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f)`：

- **相机在原点朝 `+z`**：物体要放在 `z > near` 的地方才看得见，把立方体推到 `z = 2.5` 等价于"相机后退到离物体 2.5 的位置"
- **立方体占据 `z ∈ [2.25, 2.75]`**：两端都落在 `[0.1, 10]` 之内，不会被近裁剪面或远裁剪面切掉
- **50° 视场角在 `z = 2.5` 处的半高是 `2.5 * tan(25°) ≈ 1.17`**：半宽是它的 `1.33` 倍，约 `1.55`；边长 0.5 的立方体只占半高的 `0.25 / 1.17 ≈ 21%`，所以看起来是画面中央一个不大的方块
- **缩放 0.5 在这里不是"变小"的唯一原因**：距离把物体在屏幕上的尺寸压小，这才是透视投影与正交投影的差别——同样大小的物体，放得越远画得越小

深度值可以直接算出来（`near = 0.1`、`far = 10`）：

| 相机空间 z | NDC 深度 |
| :--: | :--: |
| 0.1 (near) | 0.000 |
| 2.25 (立方体前表面) | 0.965 |
| 2.50 (立方体中心) | 0.970 |
| 2.75 (立方体后表面) | 0.973 |
| 10 (far) | 1.000 |

- **整个立方体只占 `0.965 ~ 0.973` 这一小段**：深度缓冲里 36 个顶点的值几乎挤在一起，这正是 `f/n = 100` 带来的非线性压缩
- **所以 far 不能设太大**：把 `far` 再放大十倍，这段区间还会继续向 1 收缩，立方体自身的六个面就可能开始深度冲突 (Z-Fighting)
- **顺序仍然靠深度测试**：`VK_COMPARE_OP_LESS` 配清屏值 1.0，NDC 深度更小的面画在前面，背面被挡住，立方体才有实心感
