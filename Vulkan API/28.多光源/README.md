# 多光源

在之前的[点光源](../25.点光源/README.md)一节中，我们通过一个 `GlobalUbo` 向着色器传递点光源的信息。

目前点光源的渲染分为两步：

1. 为场景中的所有模型计算光照
   - 绘制模型时只传入一个点光源的数据，因此所有模型都只受这一个光源影响
2. 将点光源本身绘制出来

这两步都建立在「场景中只有一个点光源」的前提之上，本章要做的就是把这一前提推广到多个点光源

## 光源数组

`frame_info.hpp` 里定义了光源数量上限、单个光源的字段，以及包含光源数组的 UBO：

```cpp
#define MAX_LIGHTS 10

struct PointLight {
    glm::vec4 position{}; // ignore w
    glm::vec4 color{}; // w is intensity
};

struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
  PointLight pointLights[MAX_LIGHTS];
  int numLights;
};
```

- **`MAX_LIGHTS` 是预处理宏，值为 10**：数组长度与断言上限都引用它
- **`PointLight` 由两个 `vec4` 组成**：`position` 的 `w` 不使用，`color` 的 `w` 存放强度
- **`GlobalUbo` 有五个字段**：`projection`、`view`、`ambientLightColor`、`pointLights`、`numLights`
- **`numLights` 是 `int`**：记录当帧实际填入的光源个数
- **`GlobalUbo` 定义在 `frame_info.hpp`**：`PointLightSystem::update` 以 `GlobalUbo&` 参数接收它，类型需要在这里可见
- **数组元素按 std140 步长 32 字节排列**：每个 `PointLight` 的两个 `vec4` 各占 16 字节
- **实测偏移是 0、64、128、144、464**：依次对应 `projection`、`view`、`ambientLightColor`、`pointLights`、`numLights`
- **`sizeof(GlobalUbo)` 为 468**：std140 的块大小按 16 字节取整是 480，末尾 12 字节没有任何字段，着色器不会读取
- **UBO 缓冲区按 `sizeof(GlobalUbo)` 分配**：`MyBuffer` 构造时传入的就是这个值

四个着色器都声明了同一份 `GlobalUbo`，数组长度写成字面量 `10`：

```glsl
struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor; // w is intensity
    PointLight pointLight[10];
    int numLights;
} ubo;
```

- **数组长度是字面量 `10`**：GLSL 不经过预处理，`MAX_LIGHTS` 不会出现在着色器里
- **字段名是 `pointLight`（单数）**：C++ 侧写作 `pointLights`，名字差异只影响着色器里的访问写法，不改变内存布局
- **`numLights` 声明为 `int`**：循环上界直接取这个字段
- **字段顺序与 C++ 侧逐项对应**：`projection`、`view`、`ambientLightColor`、`pointLight`、`numLights`
- **`simple_shader.vert`、`simple_shader.frag`、`point_light.vert`、`point_light.frag` 都带这份声明**：四个着色器共用 `set = 0, binding = 0`

## 光源对象

`GameObject` 用一个组件标记自己是光源，并记录强度：

```cpp
struct PointLightComponent {
    float lightIntensity = 1.0f;
};

class GameObject {
    ...
    std::unique_ptr<PointLightComponent> pointLight{};
};
```

- **`pointLight` 为空即不是光源**：各处用 `if (obj.pointLight == nullptr) continue;` 做判据
- **`lightIntensity` 默认 1.0f**：`makePointLight` 会按传入的 `intensity` 覆盖它
- **组件用 `unique_ptr` 持有**：`GameObject` 移动时组件随之转移

工厂函数把半径写进缩放分量：

```cpp
static GameObject makePointLight(
    float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(0.f));
```

```cpp
GameObject GameObject::makePointLight(float intensity, float radius, glm::vec3 color)
{
    GameObject gameObj = GameObject::createGameObject();
    gameObj.color = color;
    gameObj.transform.scale.x = radius;
    gameObj.pointLight = std::make_unique<PointLightComponent>();
    gameObj.pointLight->lightIntensity = intensity;
    return gameObj;
}
```

- **三个默认参数**：`intensity = 10.f`、`radius = 0.1f`、`color = glm::vec3(0.f)`
- **半径存在 `transform.scale.x`**：广告牌的尺寸由这个分量决定
- **返回值按值传递**：`GameObject` 的移动构造可用，拷贝构造被 `delete`
- **对象由 `createGameObject` 生成**：`id` 在静态计数器上自增

`loadGameObjects` 里放了六个光源，颜色取自一张固定表：

```cpp
std::vector<glm::vec3> lightColors{
    {1.f, .1f, .1f},
    {.1f, .1f, 1.f},
    {.1f, 1.f, .1f},
    {1.f, 1.f, .1f},
    {.1f, 1.f, 1.f},
    {1.f, 1.f, 1.f}
};

for (int i = 0; i < lightColors.size(); i++) {
  auto pointLight = GameObject::makePointLight(0.2f);
  pointLight.color = lightColors[i];
  auto rotateLight = glm::rotate(
    glm::mat4(1.f),
    (i * glm::two_pi<float>()) / lightColors.size(),
    {0.f, -1.f, 0.f});

  pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
  gameObjects.emplace(pointLight.getID(), std::move(pointLight));
}
```

- **光源个数等于 `lightColors` 的长度**：六个颜色对应六个光源
- **强度统一传 `0.2f`**：调用只给出第一个参数
- **颜色逐个覆盖**：`makePointLight` 的默认颜色是 `vec3(0.f)`，随后按索引赋 `lightColors[i]`
- **初始位置绕原点等角分布**：`(i * 2π) / 6` 弧度绕 `(0, -1, 0)` 旋转基向量 `(-1, -1, -1)`
- **半径取 `radius` 的默认值**：调用没有传第二个参数
- **对象以自身 `id` 为键入表**：`emplace(pointLight.getID(), std::move(pointLight))`

## 每帧的光源收集

`PointLightSystem::update` 遍历场景对象，把光源的位置与颜色写进 UBO 数组：

```cpp
void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
{
  auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, {0.f, -1.f, 0.f});
  int lightIndex = 0;
  for (auto& kv : frameInfo.gameObjects)
  {
    auto& obj = kv.second;
    if (obj.pointLight == nullptr) continue;

    assert(lightIndex < MAX_LIGHTS && "Point light index out of range");

    // update light position
    obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));

    // copy light to ubo
    ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
    ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

    lightIndex += 1;
  }

  ubo.numLights = lightIndex;
}
```

- **旋转角取 `frameInfo.frameTime`**：每帧绕 `(0, -1, 0)` 转 `frameTime` 弧度，角度在帧间累积
- **`obj.transform.translation` 被就地改写**：下一帧的旋转从这个新位置继续
- **数组下标按遍历顺序递增**：`gameObjects` 是 `unordered_map`，填入顺序由哈希表的迭代顺序决定
- **`assert` 拦住超出 `MAX_LIGHTS` 的光源**：`lightIndex` 不会写出数组边界
- **`position` 的 `w` 填 1.0**：与顶点着色器里 `vec4(position, 1.0)` 的用法一致
- **`color` 的 `w` 填 `lightIntensity`**：强度随颜色一并传入着色器
- **`numLights` 在循环结束后写入**：值等于实际填入的个数，可以小于 `MAX_LIGHTS`
- **调用位置在 `writeToBuffer` 之前**：`ubo.projection` 与 `ubo.view` 赋值之后、`uboBuffers[frameIndex]->writeToBuffer(&ubo)` 之前

## 着色器中的多光源累加

片元着色器逐个光源计算漫反射并累加：

```glsl
void main() {
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLight[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0.0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;
    }

    outColor = vec4(diffuseLight * fragColor, 1.0); // RGBA比例
}
```

- **`diffuseLight` 的初值是环境光**：`ambientLightColor.xyz * ambientLightColor.w`，只在循环外算一次
- **循环上界是 `ubo.numLights`**：数组里未被填写的槽位不参与计算
- **`surfaceNormal` 在循环外归一化一次**：循环内每个光源复用同一个法线
- **每个光源独立算方向、衰减、入射角**：三项都基于该光源自己的 `position` 与 `color`
- **`intensity` 合并颜色与强度**：`light.color.xyz * light.color.w * attenuation`
- **结果累加进 `diffuseLight`**：多个光源的漫反射项相加
- **`outColor` 只乘 `fragColor`**：环境光已经并进 `diffuseLight` 的初值
- **`fragPosWorld` 与 `fragNormalWorld` 来自顶点阶段的插值**：光照逐片段求值

## 光源广告牌的推送常量

每个光源的广告牌位置、颜色、半径通过推送常量传入：

```cpp
struct PointLightPushConstants {
  glm::vec4 position{};
  glm::vec4 color{};
  float radius{};
};
```

- **`sizeof(PointLightPushConstants)` 为 36**：两个 `vec4` 共 32 字节，`float` 占 4 字节
- **三个字段与着色器里的 `Push` 块对应**：`vec4 position`、`vec4 color`、`float radius`
- **`position` 与 `color` 的 `w` 都有用途**：`position` 的 `w` 填 1.0，`color` 的 `w` 填强度

管线布局为它开出一个范围：

```cpp
VkPushConstantRange pushConstantRange{};
pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
pushConstantRange.offset = 0;
pushConstantRange.size = sizeof(PointLightPushConstants);

VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
pipelineLayoutInfo.pSetLayouts = setLayouts.data();
pipelineLayoutInfo.pushConstantRangeCount = 1;
pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
```

- **范围从偏移 0 开始、长度为 36**：与结构体大小一致
- **两个阶段都开放**：顶点阶段读 `position` 与 `radius`，片段阶段读 `color`
- **`pushConstantRangeCount` 为 1**：这条管线只有一个推送常量范围

`render` 里描述符集只绑定一次，然后逐个光源推送并绘制：

```cpp
for (auto& kv : frameInfo.gameObjects)
{
  auto& obj = kv.second;
  if (obj.pointLight == nullptr) continue;

  PointLightPushConstants push{};
  push.position = glm::vec4(obj.transform.translation, 1.f);
  push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
  push.radius = obj.transform.scale.x;

  vkCmdPushConstants(
    frameInfo.commandBuffer,
    pipelineLayout,
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    0,
    sizeof(PointLightPushConstants),
    &push);

  vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}
```

- **描述符集绑定在循环之前**：`set = 0`、`descriptorSetCount = 1`、`dynamicOffsetCount = 0`
- **每个光源一次推送加一次绘制**：`vkCmdPushConstants` 与 `vkCmdDraw` 成对出现
- **`push.position` 取 `obj.transform.translation`**：与写进 UBO 的位置是同一个值
- **`push.radius` 取 `obj.transform.scale.x`**：广告牌半径来自缩放分量
- **`vkCmdDraw` 每次画 6 个顶点**：六顶点构成一张方形广告牌，调用次数等于光源个数
- **没有 `vkCmdBindVertexBuffers`**：顶点数据来自 `OFFSETS` 常量数组

顶点着色器用 `push` 生成广告牌四角：

```glsl
vec3 positionWorld = push.position.xyz
    + push.radius * fragOffset.x * cameraRightWorld
    + push.radius * fragOffset.y * cameraUpWorld;

gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
```

- **广告牌圆心是 `push.position.xyz`**：每个光源各自的位置
- **半径是 `push.radius`**：四角到圆心的偏移由它缩放
- **`ubo` 仍负责 `projection` 与 `view`**：`point_light.vert` 声明了 `GlobalUbo` 并只读取这两个字段

片元着色器用 `push.color` 上色：

```glsl
outColor = vec4(push.color.xyz, 1.0);
```

- **颜色取 `push.color.xyz`**：`w` 里的强度不参与显示
- **`point_light.frag` 也声明了 `GlobalUbo`**：但这段绘制只用推送常量里的字段
