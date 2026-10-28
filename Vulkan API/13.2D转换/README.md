# 2D转换

2D转换 (2D Transformation) 是在二维平面上改变图形的位置、方向、大小或形状的操作

在图形学中，这些操作统一用**矩阵乘法**来描述，原因有两点：
- 一个矩阵可以同时表达缩放、旋转、错切等多种变换，只需做一次乘法
- 多个变换可以通过矩阵连乘合成为一个矩阵，避免对每个顶点重复计算

常见的 2D 基本变换：

| 变换 | 作用 | 是否线性 |
|------|------|----------|
| 平移 (Translation) | 整体移动位置 | 否 (仿射) |
| 旋转 (Rotation) | 绕原点转动 | 是 |
| 缩放 (Scale) | 放大或缩小 | 是 |
| 错切 (Shear) | 让图形倾斜变形 | 是 |

在 Vulkan 中，顶点坐标最终由顶点着色器输出到裁剪空间 (Clip Space)，因此 2D 转换本质上就是对顶点坐标乘上一个变换矩阵，再交给光栅化

## 线性转换

线性变换 (Linear Transformation) 是满足以下两条性质的变换：

- 可加性：`T(a + b) = T(a) + T(b)`
- 齐次性：`T(k · a) = k · T(a)`

直观理解线性变换的特点：

1. **原点保持不动**：因为 `T(0) = 0`
2. **直线仍是直线**：变换后不会把直线弯成曲线
3. **平行仍平行**：原本平行的两条线变换后依然平行

二维线性变换总可以写成一个 `2 × 2` 矩阵乘法：

```
[ x' ]   [ a  b ] [ x ]
[ y' ] = [ c  d ] [ y ]
```

可以这样理解这个矩阵：它的第一列 `(a, c)` 是基向量 `(1, 0)` 变换后的结果，第二列 `(b, d)` 是基向量 `(0, 1)` 变换后的结果，所以**矩阵就是变换本身**

常见的线性变换及其矩阵：

| 变换 | 矩阵 | 说明 |
|------|------|------|
| 缩放 | `[[sx, 0], [0, sy]]` | `sx`、`sy` 为各轴缩放比例 |
| 旋转 | `[[cosθ, -sinθ], [sinθ, cosθ]]` | 逆时针旋转 `θ` |
| 错切 | `[[1, k], [0, 1]]` | 沿 x 方向剪切，`k` 为剪切系数 |

需要注意：

- **平移不是线性变换**，因为 `T(0) = 0` 不成立，它属于仿射变换 (Affine Transformation)
- 为了用统一的矩阵乘法同时表达平移、旋转、缩放，需要引入**齐次坐标 (Homogeneous Coordinates)**，把 `2 × 2` 矩阵扩展为 `3 × 3` 矩阵，把二维点 `(x, y)` 写成 `(x, y, 1)`
- 矩阵乘法**不满足交换律**，所以变换的顺序很重要：先缩放再旋转，与先旋转再缩放，结果通常不同

## 齐次坐标下的 3 × 3 矩阵

把平移也纳入矩阵乘法后，二维变换写成：

```
[ x' ]   [ a  b  tx ] [ x ]
[ y' ] = [ c  d  ty ] [ y ]
[ 1  ]   [ 0  0  1  ] [ 1 ]
```

这个矩阵左上角的 `2 × 2` 块是线性部分，最后一列的前两个分量 `(tx, ty)` 是平移量。它作用在点 `(x, y, 1)` 上，结果等价于：

```
p' = M · p + t
```

其中 `M` 是 `2 × 2` 线性变换，`t` 是平移向量。也就是说，**仿射变换 = 线性变换 + 平移**，这也是实际实现中最常用的形式：不必真的构造 `3 × 3` 矩阵，把 `M` 和 `t` 分开传入即可，省去一半以上的数据量

## 变换组件 Transform2dComponent

把平移、缩放、旋转集中在一个组件里，便于给每个物体单独设置：

```cpp
struct Transform2dComponent {
    glm::vec2 translation{};
    glm::vec2 scale{1.f, 1.f};
    float rotation;
    glm::mat2 mat2() {
        const float sin = glm::sin(rotation);
        const float cos = glm::cos(rotation);
        glm::mat2 rotMatrix = {{cos, sin}, {-sin, cos}};
        glm::mat2 scaleMat{{scale.x, 0.f}, {0.f, scale.y}};
        return rotMatrix * scaleMat;
    }
};
```

几个关键点：

1. **glm 矩阵是列优先的**：`{{cos, sin}, {-sin, cos}}` 中的两个花括号分别对应第一列和第二列，所以它实际表示的矩阵是 `[[cos, -sin], [sin, cos]]`，即绕原点逆时针旋转 `rotation` 弧度。如果按行优先的直觉去写，旋转方向会反过来
  - 在 Vulkan 中，矩阵是按照列排序的
  - 例如要组 `[[a, b], [c, d]]`
  - 就需要写成 `[[a, c], [b, d]]`
2. **`rotation` 的单位是弧度**：代码开头定义的 `GLM_FORCE_RADIANS` 保证 `glm::sin` / `glm::cos` 按弧度计算，`glm::two_pi<float>()`、`glm::pi<float>()` 来自 `<glm/gtc/constants.hpp>`
3. **组合顺序**：`rotMatrix * scaleMat` 表示先缩放、再旋转。因为矩阵乘法从右往左作用于向量，`(R · S) · p` 等价于 `R · (S · p)`；反过来写 `scaleMat * rotMatrix` 就是先旋转再缩放，结果不同
4. **旋转与缩放都绕原点**：`2 × 2` 线性变换无法表达"绕图形中心旋转"，需要先把图形平移到原点附近，或者把旋转中心也写进齐次矩阵

## 扩展后的推送常量

变换数据要送到着色器，推送常量的结构体相应扩展：

```cpp
struct SimplePushConstantsData {
  glm::mat2 transform{1.f};
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};
```

- `glm::mat2 transform{1.f}` 的默认值是把对角元素填成 `1.f`，即单位矩阵，未设置变换时顶点保持原样
- `offset` 单独承担平移，`transform` 只承担线性部分，两者在着色器中按 `transform * position + offset` 组合，正好对应上面的 `M · p + t`

## 着色器中的变换

顶点着色器声明与 C++ 结构体对应的推送常量块，并把线性变换与平移组合起来：

```glsl
layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main() {
    gl_Position = vec4(push.transform * position + push.offset, 0.0, 1.0);
}
```

要点：

- `push.transform * position` 先对顶点做缩放、旋转（以及可能的错切），再加 `push.offset` 完成平移，等价于前面 `3 × 3` 齐次矩阵的完整效果
- 顶点着色器输出的已经是**裁剪空间**坐标，所以平移量的单位是裁剪空间单位：`offset = 1.0` 相当于移动半个屏幕宽/高，`[-1, 1]` 之外的顶点会被裁掉
- 片段着色器也要声明**完全一致**的推送常量块（成员顺序、类型都相同）：

```glsl
layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;
```

两个阶段共用同一段推送常量内存，若片段着色器只声明 `vec2 offset; vec3 color;`，它会按自己的声明去解析这块内存，把 `transform` 的前 8 字节当成 `offset`，读到完全错误的颜色
- 片段着色器只用到 `push.color`，但仍必须声明完整块，因为成员偏移是由声明顺序决定的

## 游戏对象 GameObject

把"模型 + 颜色 + 变换"打包成一个对象，渲染循环就可以统一遍历处理：

```cpp
class GameObject {
    public:
        using id_t = unsigned int;
        static GameObject createGameObject() {
            static id_t current_id = 0;
            return GameObject(current_id++);
        }
        GameObject(const GameObject &) = delete;
        GameObject(GameObject &&) = default;
        GameObject &operator=(const GameObject &) = delete;
        GameObject &operator=(GameObject &&) = delete;
        id_t getID() { return id; }
        std::shared_ptr<MyModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d{};
    private:
        id_t id;
        GameObject(id_t obj_id) : id(obj_id) {}
};
```

设计要点：

- **唯一 id**：静态计数器在每次 `createGameObject()` 时自增，保证每个对象有稳定且不重复的标识，后续做对象查找、删除、缓存时都能用上
- **只允许移动、禁止拷贝**：拷贝会让两个对象拥有同一个 id，语义上就不再是独立对象；移动构造被保留，因此可以放进 `std::vector`（容器扩容时会移动元素）
- **构造函数私有**：只能通过 `createGameObject()` 创建，从而强制走 id 分配逻辑
- **`std::shared_ptr<MyModel>`**：模型持有顶点缓冲和索引缓冲，用共享指针可以让多个对象复用同一个模型，避免同一份网格在显存里被重复创建
- `transform2d` 与 `color` 是每个对象各自的"可变状态"，每帧推送常量时从这里取值

## 场景初始化

创建对象、绑定模型并设置初始变换：

```cpp
void FirstApp::loadGameObjects() {
  std::vector<MyModel::Vertex> vertices {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  auto model = std::make_shared<MyModel>(device, vertices);

  auto triangle = GameObject::createGameObject();
  triangle.model = model;
  triangle.color = {0.1f, 0.8f, 0.1f};
  triangle.transform2d.translation.x = .2f;
  triangle.transform2d.scale = {2.f, .5f};
  triangle.transform2d.rotation = .25f * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}
```

这里 `.25f * glm::two_pi<float>()` 就是 `90°`（四分之一圈）对应的弧度值。注意顶点属性里的 `color` 此时已不参与计算——颜色完全来自对象的 `color` 成员，通过推送常量传给片段着色器

## 渲染循环

所有对象的绘制被收进 `renderGameObjects`：

```cpp
void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
  pipeline->bind(commandBuffer);

  for (auto& obj : gameObjects) {
    obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.1f, glm::pi<float>());

    SimplePushConstantsData push = {};
    push.offset = obj.transform2d.translation;
    push.color = obj.color;
    push.transform = obj.transform2d.mat2();

    vkCmdPushConstants(
      commandBuffer,
      pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(push),
      &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}
```

要点：

- **管线只绑定一次**：所有对象共用同一个管线和管线布局，循环里只重复"推送常量 + 绑定模型 + 绘制"
- **动画在录制时更新**：每帧录制命令缓冲区时给 `rotation` 累加 `0.1` 弧度，于是三角形在每帧之间持续转动
- **`glm::mod` 限制角度范围**：`glm::mod(rotation, glm::pi<float>())` 把角度收在 `[0, π)` 内，防止浮点数随运行时间无限增大而逐渐丢失精度；因为三角形旋转 `π` 后与自身重合，这种回绕在视觉上不会产生跳变
- **逐对象推送常量**：`offset`、`transform`、`color` 每个对象都不同，所以同一个模型能以不同位置、不同姿态、不同颜色绘制多次，无需额外的 UBO 或描述符集
- **绘制顺序**：`push` 在 `draw` 之前推送，GPU 执行到绘制命令时读取的是当时最新的常量值

需要注意的一点：由于没有投影矩阵，`transform` 直接作用于裁剪空间，缩放效果会被窗口宽高比拉伸——例如 `scale = {2.f, .5f}` 在非正方形窗口中的观感并不等于"宽是高的 4 倍"，只有在方形视口中才是严格的几何比例
