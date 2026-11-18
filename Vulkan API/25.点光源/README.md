# 点光源

点光源 (Point Light) 是位于空间中某一个具体位置、向所有方向均匀发光的光源

现实中的灯泡、火把、蜡烛、萤火虫都可以用点光源来近似

与平行光不同，点光源具有明确的世界坐标，光线方向取决于「着色点指向光源」的方向，并且亮度会随着距离增加而衰减 (attenuation)

因此描述一个点光源通常需要三类信息：

- 位置：光源在世界空间中的坐标
- 颜色与强度：光源本身的颜色和发光强度
- 衰减参数：常数项、一次项、二次项系数，用于控制亮度随距离下降的速度

## 与传统漫反射光源的比较

传统漫反射光照 (本系列此前使用的平行光 / 方向光近似) 假设光源位于无穷远处，到达场景中每一点的光线方向都相同，因此只需要一个归一化的方向向量即可

它的漫反射项可以概括为「环境光 + 法线与光线方向夹角的余弦」，亮度不随距离变化

两者的主要差异如下：

| 对比项 | 传统漫反射光源 (平行光) | 点光源 |
| :---: | :------------------: | :---: |
| 光源位置 | 无穷远，无具体坐标 | 世界空间中的具体坐标 |
| 光线方向 | 全局统一，处处相同 | 每个着色点都不同，等于「光源位置 − 着色点位置」的归一化方向 |
| 距离衰减 | 无，亮度与距离无关 | 有，通常按 `1 / (1 + k·d + k·d²)` 或物理上的平方反比衰减 |
| 计算位置 | 顶点着色器即可（方向不变，插值后仍正确） | 一般在片元着色器逐片元计算（方向逐像素变化，顶点插值会产生误差） |
| 参数数量 | 少，只需方向、颜色、环境光 | 多，需要位置、颜色、衰减系数等 |
| 视觉效果 | 均匀、平坦，适合模拟太阳光/月光等远距离光源 | 局部感强，能表现灯泡照亮周围、明暗过渡等效果 |
| 性能开销 | 低 | 较高，尤其多光源时每个片元都要计算距离与衰减 |

- 传统漫反射光源是**对「极远距离光源」的简化**，计算便宜但缺乏空间感
- 点光源则**引入了位置与距离衰减**，用更高的计算成本换取更真实、更有局部氛围的光照表现

两者并不是互相替代的关系，实际渲染中常同时使用：平行光提供整体照明基调，点光源负责局部亮点与氛围

## 统一缓冲区里的光源字段

`GlobalUbo` 携带一整套点光源参数：

```cpp
struct GlobalUbo {
  glm::mat4 ProjectionMatrix{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
  glm::vec3 lightPosition{-1.f};
  alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity, 
  // vec4 lightColor = (r, g, b, intensity) => (r * intensity, g * intensity, b * intensity)
};
```

- **`ProjectionMatrix` 是 `P * V`**：相机相关的那一份逐帧重算
- **`ambientLightColor` 用 `vec4` 装颜色与强度**：前三个分量是 RGB，`w` 是强度系数，实测初始值为 `(1, 1, 1, 0.02)`
- **`lightPosition` 是 `vec3`**：光源的世界坐标，实测初始值为 `(-1, -1, -1)`
- **`lightColor` 的 `w` 是光源强度**：前三个分量是颜色，实测初始值为 `(1, 1, 1, 1)`
- **单标量初始化会把所有分量填成同一个值**：`glm::vec3{-1.f}` 得到 `(-1, -1, -1)`，`glm::vec4{1.f}` 得到 `(1, 1, 1, 1)`
- **`alignas(16)` 是必需的**：GLM 默认 `alignof(glm::vec4)` 只有 4，去掉这个声明后 `lightColor` 会落到偏移 92、结构体缩到 108 字节，与 std140 要求的偏移 96 不符
- **颜色与强度打包进 `vec4` 的 `w`**：省下一个独立字段，也避免 std140 里多出来的对齐空隙
- **注释里写明了拆分方式**：`(r, g, b, intensity)` 与 `(r * intensity, g * intensity, b * intensity)` 是同一组数据的两种写法

## 字段的偏移与对齐

实测 `sizeof(GlobalUbo)` 为 112、`alignof` 为 16，四个字段的偏移分别是：

| 字段 | 类型 | 偏移 | 长度 |
| :---: | :---: | :---: | :---: |
| `ProjectionMatrix` | `mat4` | 0 | 64 |
| `ambientLightColor` | `vec4` | 64 | 16 |
| `lightPosition` | `vec3` | 80 | 12 |
| `lightColor` | `vec4` | 96 | 16 |

- **`lightPosition` 之后有 4 字节空隙**：`vec3` 本身只占 12 字节，但下一个 `vec4` 必须落在 16 的倍数上，96 − (80 + 12) = 4
- **std140 的对齐规则与这套偏移完全吻合**：`vec3` 按 16 对齐（偏移 80 是 16 的倍数），`vec4` 按 16 对齐（偏移 96 也是）
- **C++ 侧的 112 字节就是着色器侧块的总长度**：112 本身是 16 的倍数，std140 尾部补全不再追加字节
- **描述符范围覆盖整个 112 字节**：`descriptorInfo()` 默认 `VK_WHOLE_SIZE`，`instanceSize` 就是 `sizeof(GlobalUbo)`
- **没有跨字段的重叠**：`lightPosition` 的 4 字节空隙是纯填充，着色器不会读到它

## 每帧写入

帧循环里只重算投影视图矩阵，其余三个字段保持默认值：

```cpp
GlobalUbo ubo{};
ubo.ProjectionMatrix = camera.getProjectionMatrix() * camera.getViewMatrix();
uboBuffers[frameIndex]->writeToBuffer(&ubo);
uboBuffers[frameIndex]->flush();
```

- **`GlobalUbo ubo{}` 每帧重新构造**：四个默认成员初始化器都会重新执行一遍
- **只有 `ProjectionMatrix` 被赋值**：`ambientLightColor`、`lightPosition`、`lightColor` 都停在默认值
- **因此光源在整个运行期间不动**：位置固定在 `(-1, -1, -1)`，颜色固定为 `(1, 1, 1)`、强度 1，环境光强度固定为 `0.02`
- **`writeToBuffer(&ubo)` 拷 112 字节**：不带 size 时按 `bufferSize` 整块 `memcpy`，即 `sizeof(GlobalUbo)`
- **`flush()` 必需**：缓冲区内存属性只有 `HOST_VISIBLE`，没有 `HOST_COHERENT`
- **写入发生在 `beginSwapChainRenderPass` 之前**：本帧的 `P * V` 与光源参数在录制绘制命令前就到位

## 着色器里的统一缓冲区块

顶点着色器声明四个字段，顺序与 C++ 结构一致：

```glsl
layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec4 ambientLightColor; // w is intensity
    vec3 lightPosition;
    vec4 lightColor;
} ubo;
```

- **`projectionViewMatrix` 对应 C++ 的 `ProjectionMatrix`**：这是块里唯一名称不同的字段
- **其余三个字段两边同名**：`ambientLightColor`、`lightPosition`、`lightColor` 逐字对应
- **字段顺序必须一致**：std140 只按声明顺序与类型计算偏移，顺序错位读到的就是别的字段
- **`lightColor` 的 `w` 在着色器里被当作强度**：与 C++ 侧注释的约定一致
- **块位于 `set = 0, binding = 0` 这一处**：光源参数与 `P * V` 共用同一份全局统一缓冲区
- **布局里的 `stageFlags` 是 `VK_SHADER_STAGE_VERTEX_BIT`**：块只在顶点阶段被读取，片元着色器没有声明它

## 世界空间位置

顶点着色器先把顶点变换到世界空间，再算光照：

```glsl
vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);

gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);
```

- **`positionWorld` 只做 `M`**：不含投影与视图矩阵，得到的就是世界坐标
- **`gl_Position` 另外做了一次完整的变换**：`P * V * M` 直接写在表达式里
- **`push.modelMatrix` 被用了两次**：两次乘法各自独立，编译器可以复用同一个中间结果
- **`positionWorld` 只在光照计算里使用**：方向向量必须建立在世界空间，否则光源位置与顶点位置不在同一坐标系
- **光照在顶点着色器里算**：`fragColor` 是顶点输出，经过光栅化插值后写入片元

### 为什么先把顶点变换到世界空间

根本原因一句话：**减法、点积、距离这些运算只在同一坐标系内才有意义**。顶点的原始坐标在模型空间，光源位置在世界空间，两者混在一起算，得到的数值既不是方向也不是距离。下面用本场景的具体数据说明"错在哪、错多少"

**1. 模型空间的坐标到底是什么**

`smooth_vase.obj` 里的顶点坐标以模型自身原点为中心，数值大致落在 `-1 ~ +1` 之间。这套坐标与物体在场景里的位置、大小、朝向完全无关——它只是"这个花瓶自己长什么样"的描述。而 `lightPosition` 是世界空间里的 `(-1, -1, -1)`，是"灯在场景里的哪个位置"。前者是物体自己的尺子，后者是场景的尺子，两者没有共同的刻度

**2. 跳过模型变换，距离会错多少**

本场景里物体的变换是：平移 `(0, 0.5, 2.5)`、缩放 `3`。一个局部坐标为 `(0, 0, 0)` 的顶点，它的真实世界位置是 `3 * (0,0,0) + (0, 0.5, 2.5) = (0, 0.5, 2.5)`。到光源 `(-1, -1, -1)` 的距离：

- 正确：`Δ = (1, 1.5, 3.5)`，`d² = 1 + 2.25 + 12.25 = 15.5`，衰减 `1/15.5 ≈ 0.065`
- 跳过变换：顶点停在 `(0, 0, 0)`，`Δ = (1, 1, 1)`，`d² = 3`，衰减 `1/3 ≈ 0.333`

差了约 **5.2 倍**。这不是"略有偏差"，而是整块表面被错误地当成离光源近得多，亮度直接翻了几倍

**3. 缩放会整体压缩距离，衰减误差是平方级**

如果物体正好在世界原点、缩放 3，那么不做变换时所有距离都被压缩到 1/3，`d²` 变成真实值的 1/9，衰减就放大了 **9 倍**。缩放越大，错得越离谱——而且这种错误在视觉上表现为"物体越大越亮"，很容易被误认为"光照正常"

**4. 方向向量同样会错，不只是距离**

`lightPosition - positionWorld` 这个减法同时决定了**方向**和**距离**。顶点位置错了，方向自然错，`dot(normal, directionToLight)` 也随之错，明暗分界线（terminator）会跑到错误的位置。所以错误不止体现在亮度上，还体现在"哪一面被照亮"上

**5. 法线：为什么还要单独的法线矩阵**

法线是方向量，随物体变换一起改变，但它**不能直接乘模型矩阵**。取本场景的非等比缩放 `(3, 1, 1)`：一个表面法线 `(1, 1, 0)` 和它的切向量 `(1, -1, 0)` 本来互相垂直。乘模型矩阵后法线变成 `(3, 1, 0)`、切向变成 `(3, -1, 0)`，两者点积为 `9 - 1 = 8`，**不再垂直**——法线被"拉歪"了。改用逆转置的法线矩阵（对 `(3,1,1)` 而言相当于 `(1/3, 1, 1)`），法线变成 `(1/3, 1, 0)`，与 `(3, -1, 0)` 的点积为 `1 - 1 = 0`，垂直关系得以保持。所以法线必须用法线矩阵（而不是模型矩阵）带到世界空间，才能与光线方向正确地做点积

**6. 为什么不用视图空间或裁剪空间**

视图空间的原点是相机，会随相机一起移动。光源在世界空间里是固定的，若在视图空间里算，就必须每帧把 `lightPosition` 用视图矩阵变换一次；一旦忘了变换，灯就会"粘"在相机上，你转身时高光跟着你转，看起来像探照灯。裁剪空间更糟：它带着投影变换，是非线性的，距离和方向都被投影扭曲，完全无法用来做物理意义上的衰减。光照是**场景的固有属性**，应该在一个不随相机变化的空间里算，世界空间正是这样的空间

**7. 为什么不干脆把光源搬进模型空间**

那样每个物体都要把光源做一次 `inverse(M) * lightPos`，而且逆变换会把距离"拉伸"：在非等比缩放下，局部空间里沿 x 轴的距离被压缩了 3 倍，沿 y 轴不变，同一个 `d²` 在不同方向上代表不同的真实距离，平方反比衰减失去物理意义。统一在世界空间比较，所有物体共用同一份光源参数，只需算一次

**8. 世界空间还与渲染流程解耦**

投影和视图矩阵的职责只是把世界空间的内容映射到屏幕，它们和光照没有关系。先在世界空间把光照算完，再交给 `P * V`，两件事互不干扰；如果混在一起，后续想改投影方式（透视/正交）都会牵动光照代码

因此，把顶点变换到世界空间，本质上是**为光照建立一个统一的、与相机无关、且距离度量不失真的参照系**，让顶点位置、光源位置、法线方向三者可以合法地放在一起做运算

## 光照计算

方向、衰减、颜色三项依次算出：

```glsl
vec3 directionToLight = ubo.lightPosition - positionWorld.xyz;
float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0.0);

fragColor = (ambientLight + diffuseLight) * color;
```

- **`directionToLight` 是「光源 − 着色点」**：指向光源，未归一化
- **`dot(directionToLight, directionToLight)` 就是距离的平方**：省掉一次 `length()` 再平方
- **`attenuation` 是平方反比**：`1 / d²`，距离加倍亮度降到四分之一
- **`lightColor` 先乘自身 `w` 再乘 `attenuation`**：颜色、强度、距离衰减三者相乘得到该点接收到的光强
- **`ambientLight` 只乘自身 `w`**：环境光不随距离变化，也不依赖法线朝向
- **`normalize(directionToLight)` 只在漫反射项里用**：点积需要单位向量，衰减项用的是未归一化的平方长度
- **`max(..., 0.0)` 把背光面钳到零**：法线与光线方向夹角超过 90 度时该光源不再贡献
- **两项相加后与顶点颜色相乘**：`color` 是顶点属性，最终写入 `fragColor`
- **`const float AMBIENT` 不在着色器里**：环境光强度来自 `ubo.ambientLightColor.w`
- **顶点法线由 `mat3(push.normalMatrix) * normal` 得到**：法线矩阵在 CPU 上算好，着色器只做一次乘法

光照只发生在顶点上，插值的粒度由模型的顶点密度决定：

- **光照按顶点算，面内线性插值**：顶点密度越低，光源正下方与远处的亮度差被插值抹平得越明显
- **插值的是颜色而不是距离**：`fragColor` 已经是颜色值，片元阶段不重算 `attenuation`
- **这份代码把整段光照放在顶点阶段**：开头对比表里「逐片元计算」的做法在这里对应的是逐顶点计算

片元着色器不参与光照计算：

```glsl
layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

- **只声明推送常量块**：没有 `set = 0` 的声明，片元阶段不读全局统一缓冲区
- **`outColor` 直接取插值后的 `fragColor`**：光照强度在顶点阶段已经乘进颜色里
- **推送常量块与顶点阶段同名同布局**：两个阶段共用同一份 128 字节范围
