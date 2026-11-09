# 游戏循环 & 用户输入

先看看没有计时机制时的表现：

- **`VK_PRESENT_MODE_MAILBOX_KHR`**：正方体旋转得非常快

  ```cpp
  if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
  ```

- **`VK_PRESENT_MODE_FIFO_KHR`**：虽说没那么夸张，但在 60Hz 与 120Hz 的屏幕上旋转速度**并不一致** (120Hz 正好快一倍) 

  ```cpp
  return VK_PRESENT_MODE_FIFO_KHR
  ```

所以需要在主循环中加入某种计时机制，让 60Hz 与 120Hz 的屏幕、以及不同的呈现模式下，旋转速度都保持一致

## 时间步

游戏引擎的基本理念是：它由多个系统组成，每个系统都能向前推进一定的时间量

这个向前推进的时间量就叫作时间步 (Timestep)，也叫时间间隔 (DT, Delta Time)，计算方法是：

```
DT = (当前时间戳 - 上一时间戳)
```

- **时间戳**：时间点的唯一标识符，通常用浮点数表示

## 量出时间步

主循环用 `std::chrono` 的高精度时钟，量出相邻两帧的间隔：

```cpp
auto currentTime = std::chrono::high_resolution_clock::now();
while (!window.shouldClose()) {
  glfwPollEvents();
  auto newTime = std::chrono::high_resolution_clock::now();
  float deltaTime =
      std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
  currentTime = newTime;

  // ...
}
```

- **`currentTime` 在循环外初始化**：进循环前先取一次时间戳，第一帧才有“上一帧”可比，`deltaTime` 近似为 0，不会凭空跳一大步
- **`newTime` 在 `glfwPollEvents()` 之后取**：事件处理 (窗口拖动、最小化) 耗掉的时间也算进这一帧的间隔，不会凭空丢失
- **`duration<float, seconds::period>`**：把两个时间点之差转成以秒为单位的 `float`，`.count()` 取出数值；直接用 `milliseconds` 还得再除 1000，多一步且有精度损失
- **`currentTime = newTime` 紧跟在计算之后**：下一帧的间隔从这一帧结束算起，不会重复计入同一段时间
- **`high_resolution_clock`**：分辨率足够高，`float` 在秒级范围内也放得下几十纳秒的差异
- **时间与渲染解耦**：即使 `renderer.beginFrame()` 返回 `nullptr`、整帧被跳过，时间照样推进，下一次能画的时候 `deltaTime` 已经把跳过的时长算进去了
- **窗口最小化期间会积累一段时间**：`recreateSwapChain()` 里阻塞在 `glfwWaitEvents()` 等待尺寸恢复，这段等待会整段计入下一帧的 `deltaTime`，恢复瞬间画面会有一次较大的跳变

## 为什么乘上 `deltaTime`

所有与运动有关的量都乘上 `deltaTime`，“每帧变化多少”于是变成“每秒变化多少”：

```cpp
gameObject.transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
gameObject.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
```

以 `moveSpeed = 3`、`lookSpeed = 1.5` 为例：

| 刷新率 | `deltaTime` | 每帧旋转 | 每帧移动 | 每秒旋转 | 每秒移动 |
| :--: | :--: | :--: | :--: | :--: | :--: |
| 60 Hz | ≈ 0.0167 s | 0.025 rad ≈ 1.43° | 0.05 | 1.5 rad ≈ 85.9° | 3 |
| 120 Hz | ≈ 0.0083 s | 0.0125 rad ≈ 0.72° | 0.025 | 1.5 rad ≈ 85.9° | 3 |
| 144 Hz | ≈ 0.0069 s | ≈ 0.0104 rad ≈ 0.60° | ≈ 0.021 | 1.5 rad ≈ 85.9° | 3 |

- **每帧的量随刷新率减半，每秒的量不变**：这正是开头“60Hz 与 120Hz 旋转速度差一倍”的解法——原先每帧固定加 `0.1` 弧度，帧率翻倍速度就翻倍
- **呈现模式也不再影响速度**：`MAILBOX` 跑得再快，每帧也只推进 `deltaTime` 那么多，累计到每秒仍是同一个值
- **`deltaTime` 天然适应卡顿**：某一帧特别慢，位移和旋转就相应变大，物体走过的时间跨度依旧正确
- **速度参数的含义变清晰**：`moveSpeed` 是“单位/秒”、`lookSpeed` 是“弧度/秒”，与刷新率无关，调参时不用再考虑帧率

## 暴露窗口句柄

输入系统要向 GLFW 查询按键状态，因此窗口把底层句柄暴露出来：

```cpp
GLFWwindow* getWindow() const { return window; }
```

- **返回裸指针而非包装类型**：`glfwGetKey` 只认 `GLFWwindow*`，直接给出原句柄最省事
- **`const` 成员函数**：只是把指针交出去，不修改 `VulkanWindow` 自身的状态
- **窗口对象仍然独占创建与销毁**：外部拿到指针只能查询按键、不能决定生命周期，`VulkanWindow` 析构时 `glfwDestroyWindow` 依旧由它自己调用

## 键盘控制

`KeyboardControl` 把“按键 → 相机运动”的映射集中在一处：

```cpp
class KeyboardControl {
    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void moveInPlaneXZ(GLFWwindow* window, float deltaTime, GameObject& gameObject);

        KeyMappings keys{};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
};
```

| 按键 | 作用 | 按键 | 作用 |
| :--: | :-- | :--: | :-- |
| `A` / `D` | 左移 / 右移 | `W` / `S` | 前进 / 后退 |
| `E` / `Q` | 上升 / 下降 | `←` / `→` | 左右转头 |
| `↑` / `↓` | 抬头 / 低头 | | |

- **键位存成 `int` 而不是枚举**：`glfwGetKey` 的参数就是 `int`，省去来回转换；`keys{}` 是公开成员，运行时可以改键位
- **`moveSpeed` 与 `lookSpeed` 公开可调**：移动 3 单位/秒、转头 1.5 弧度/秒，调节手感时不必改代码里的常数
- **`moveInPlaneXZ` 的名字已经说明了能力边界**：只在 XZ 平面 (水平面) 内移动，不会沿视线方向“飞行”
- **修改的是传入的 `GameObject`**：控制逻辑不直接碰相机，只摆弄一个普通对象的 `transform`，再由应用层把该对象的位置姿态交给相机

### 旋转：收集方向再归一化

```cpp
glm::vec3 rotate{0};

if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;

if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
}

gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
```

- **左右键写 `rotate.y`、上下键写 `rotate.x`**：`rotation.y` 是偏航 (yaw)、`rotation.x` 是俯仰 (pitch)，与 [15.欧拉角](../15.欧拉角和齐次坐标/README.md#变换组件) 的 Y-X-Z 轴序一致
- **两个方向键同时按下会互相抵消**：`+1` 与 `-1` 相加为零，`rotate` 退化成零向量，这一帧不旋转
- **`glm::normalize(rotate)` 消除斜向加速**：只按右键时 `rotate` 是 `(0, 1, 0)`，右和上同按则是 `(1, 1, 0)`，长度为 `√2`；归一化后两种情况的角速度相同，斜向转不会更快
- **[`epsilon`](#epsilon极小的容差阈值) 判断是为了防除零**：`normalize` 对零向量会得到 `NaN`，而 `dot(rotate, rotate)` 等于长度平方，用它判断比开方更省；`epsilon` 的确切含义见下文 [epsilon：极小的容差阈值](#epsilon极小的容差阈值)
- **俯仰夹在 `[-1.5, 1.5]` 弧度 (约 ±86°) **：接近 ±90° 时视线与世界上方向平行，`cross(w, up)` 趋近零向量，视图矩阵会退化，留出一点余量就不会翻面
- **偏航按 `two_pi` 取模**：一圈 2π，转过头之后数值回到小范围，长时间游玩不会累积到让 `float` 精度变差的程度
- **不处理滚转 (roll)**：`rotation.z` 始终保持 0，画面不会倾斜

### epsilon：极小的容差阈值

浮点数不是连续的，相邻两个可表示值之间总有间隙。这个“最小刻度”有正式名字：**ulp** (unit in the last place，最低位的一个单位)，即某个数值处相邻两个可表示 float 之间的间距。对 `float`，它近似为 `ulp(x) ≈ |x| * 2^-23`——**数值越大，最小单位越大**，随量级线性缩放：

| 数值所在范围 | ulp | 备注 |
| :-- | :-- | :-- |
| ≈ 1.0 | 2<sup>-23</sup> ≈ **1.19e-7** | 即 `epsilon` |
| ≈ 0.5 | 2<sup>-24</sup> ≈ 5.96e-8 | 比 `epsilon` 小一半 |
| ≈ 0.001 | ≈ 1.19e-10 | 比 `epsilon` 小一千倍 |
| ≈ 1000 | ≈ 1.19e-4 | 比 `epsilon` 大一千倍 |
| ≈ 1e6 | ≈ 0.119 | 小数点后基本没精度了 |
| ≈ 0 | ≈ 1.40e-45 | 即 `denorm_min()`，非规格化区 |

`epsilon` (希腊字母 ε) 在数学里表示“任意小的正数”，在计算机里则被借用为一个极小的容差阈值。而 `std::numeric_limits<float>::epsilon()` 就是 **ulp(1.0)**——ulp 在 1.0 处的取值，它有个精确却容易被误解的定义：

> 使 `1.0f` 与“比它大的下一个可表示 float”不相等的最小值

```cpp
1.0f + epsilon != 1.0f        // 真
1.0f + epsilon / 2 == 1.0f    // 真，加一半就浮不上去了
```

```
epsilon = 2^-23 = 1.1920928955078125e-7    (约 1.192 × 10⁻⁷) 
```

#### 为什么恰好是 2<sup>-23</sup>

一个 `float` 占 32 位，按 IEEE 754 单精度格式被切成三段：

| 段 | 位数 | 作用 |
| :-- | :--: | :-- |
| 符号位 | 1 | 正负 |
| 指数位 | 8 | 决定数量级 (2 的多少次幂) |
| 尾数位 | 23 | 决定有效数字 |

关键在于尾数位只有 23 位，加上规格化数里隐含的前导 `1`，实际精度是 24 位二进制。而 `1.0f` 的尾数正好是 `1.000…0`，想让它变大一点点，最小的一步就是改动最低位：

```
1.00000000000000000000000₂   (1.0)
1.00000000000000000000001₂   (1.0 + 2^-23)
```

所以 `2^-23` 就是 1.0 附近能分辨的最小刻度。这顺带说明了一件容易搞混的事：`float` 能表示的最大值与最小值都由**指数位**决定 (约 ±3.4e38 与 1.18e-38) ，而 `epsilon` 由**尾数位**决定，两者是完全不同的量

- **它只是 ulp(1.0)，不是“最小正数”**：正如上文 ulp 表所示，`epsilon` 只在 1.0 附近成立；若要“最小正数”，用的是 `min()` (≈ 1.18e-38) 与 `denorm_min()` (≈ 1.40e-45)
- **`std::numeric_limits` 声明在 `<limits>` 里**：这就是 [keyboard_control.cpp](keyboard_control.cpp) 开头要 `#include <limits>` 的原因
- **为什么不直接写 `> 0.f`**：浮点运算带有微小误差，理论上为 0 的量可能算成 `±1e-8` (如 `0.1f + 0.2f - 0.3f`) ，与 0 精确比较并不可靠；`epsilon` 提供的正是这种“模糊容忍带”，判零写成 `std::abs(a) <= eps` 即可

回到这两行判断：这里比较的是**长度平方**，阈值换算回长度要开一次方，即 `|rotate| > √(1.19e-7) ≈ 3.45e-4`，长度小于约万分之三点五的向量会被当成零。严格按“长度 > epsilon”的本意应写成：

```cpp
constexpr float eps = std::numeric_limits<float>::epsilon();
if (glm::dot(rotate, rotate) > eps * eps) { /* ... */ }
```

不过本例里 `rotate` 的分量只能取 `0`、`±1`、`±2`，长度平方只可能是 `0`～`4`，两种写法完全等价；一旦输入换成鼠标增量这类连续小浮点，这个换算就真的有意义了

### 移动：由偏航角推出三个方向

```cpp
float yaw = gameObject.transform.rotation.y;
const glm::vec3 forwardDir(sin(yaw), 0.f, cos(yaw));
const glm::vec3 rightDir(forwardDir.z, 0.f, -forwardDir.x);
const glm::vec3 upDir(0.f, -1.f, 0.f);
```

三个方向向量与相机三根轴的对应关系：

| 向量 | 表达式 | 对应相机的轴 | 是否受俯仰影响 |
| :-- | :-- | :-- | :--: |
| `forwardDir` | `(sin(yaw), 0, cos(yaw))` | `w` (前) 的水平投影 | 否 |
| `rightDir` | `(cos(yaw), 0, -sin(yaw))` | `u` (右)  | 否 |
| `upDir` | `(0, -1, 0)` | `-v` (上)  | 否 |

- **`forwardDir` 只由 `yaw` 决定**：抬头低头时前进方向仍是水平的，像走路而不是飞行，这正好呼应函数名里的“inPlaneXZ”
- **`forwardDir` 与 `w` 的关系**：`setViewPosition` 里 `w = (c2*s1, -s2, c1*c2)`，把俯仰 `s2` 去掉后就得到 `(sin(yaw), 0, cos(yaw))`
- **`rightDir` 是 `forwardDir` 在水平面内转 90°**：`(z, 0, -x)` 让 `(0,0,1)` 变成 `(1,0,0)`，即正前方对应屏幕右侧
- **`upDir` 取世界坐标的 `-y`**：屏幕向上对应世界的负 y (Vulkan NDC 里 y 向下) ，所以按 `E` 是“往上抬”
- **三个向量都不随俯仰变化**：抬头时按 `W` 不会向斜上方飞，`E`/`Q` 也沿世界竖直方向升降，而不是沿相机自身的上下轴

移动与旋转共用同一套“收集 + 归一化”的模式：

```cpp
glm::vec3 moveDir{0.f};
if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;

if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;

if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
}
```

- **`glfwGetKey` 是轮询**：每帧主动查一次键位状态，不需要注册回调；松开按键自然就停止移动
- **多个方向可以叠加**：`W` + `D` 斜着前进，`W` + `E` 斜着上升，归一化保证任何组合的速度都不超过 `moveSpeed`
- **`GLFW_PRESS` 表示当前按住**：`glfwGetKey` 只返回 `GLFW_PRESS` 或 `GLFW_RELEASE` 两种状态，长按不会产生额外的重复事件

## 观察者对象

相机的位置和朝向不再直接设定，改由一个普通对象转交：

```cpp
auto viewerObject = GameObject::createGameObject();
KeyboardControl cameraControl{};

while (!window.shouldClose()) {
  // ... 计算 deltaTime ...
  cameraControl.moveInPlaneXZ(window.getWindow(), deltaTime, viewerObject);
  camera.setViewPosition(viewerObject.transform.translation, viewerObject.transform.rotation);
  // ...
}
```

- **`viewerObject` 不加入 `gameObjects`**：它只当相机的数据载体，不会被渲染，也不占绘制开销
- **每帧都重新设置视图矩阵**：对象一动，`V` 就必须跟着重算；`setViewPosition` 每次都会重填 16 个元素，代价很小
- **`setViewPosition` 而不是 `setViewDirection`**：后者要自己算方向向量，前者直接吃 `translation` 与 `rotation`，正好是 `KeyboardControl` 写出的两个字段
- **与上一章的“互为逆”对应起来**：`viewerObject` 的世界变换是 `T(translation) · R(rotation)`，`setViewPosition` 产出的 `V` 就是它的逆——相机“站在这个对象的位置、朝这个对象的朝向看”，世界随之反向移动
- **相机没有位置成员**：`MyCamera` 里依旧只有两个矩阵，位置和朝向全部存在 `viewerObject` 的 `transform` 里，相机只负责“按这个姿态算出一个矩阵”
- **初始姿态由默认值决定**：`viewerObject` 的 `translation` 为 `(0,0,0)`、`rotation` 为 `(0,0,0)`，启动时相机在原点朝 +Z，与上一章 `setViewDirection((0,0,0), (0,0,1))` 的结果一致

## 绘制循环里的对象状态

渲染系统的绘制循环只剩绑定与推送：

```cpp
auto projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();

for (auto& obj : gameObjects) {
  SimplePushConstantsData push = {};
  push.color = obj.color;
  push.transform = projectionView * obj.transform.mat4();
  // ...
}
```

- **循环里只读对象状态**：`obj.transform.rotation` 不被修改，立方体的姿态完全由场景数据决定
- **画面变化来自相机而不是模型**：物体静止、相机移动，观察到的差异完全由视角产生——这正是相机变换要表达的效果
- **`projectionView` 仍在循环外计算**：`V` 每帧都变，所以这一行必须留在帧内；但它对同一帧的所有对象依然共用
- **`renderGameObjects` 的签名不变**：仍然是命令缓冲区、对象列表、相机三样东西，控制逻辑完全没有渗透进渲染系统
