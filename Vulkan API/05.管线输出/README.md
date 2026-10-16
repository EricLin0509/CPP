# 管线输出

本示例在"设备初始化"的基础上，让 `MyPipeline` 从"仅读取着色器文件"升级为"具备完整管线构建能力"。

## 管线配置信息

### PipelineConfigInfo 结构体

管线有大量可配置项（视口、裁剪矩形、着色器阶段、管线布局、渲染通道等），如果全部作为构造函数参数传递，参数列表会非常长且难以维护。因此将这些配置抽离为独立的结构体：

```cpp
struct PipelineConfigInfo {
};
```

目前结构体为空，后续会逐步填充。这样做的好处：

- 调用方只需修改关心的字段，其余使用默认值
- 同一份配置可以复用于多条管线

### defaultConfigInfo 静态工厂方法

```cpp
PipelineConfigInfo MyPipeline::defaultConfigInfo(uint32_t width, uint32_t height)
{
    PipelineConfigInfo configInfo;
    return configInfo;
}
```

接收窗口宽高，生成一份默认配置。调用方可以先拿到默认配置，再按需修改，最后传入构造函数：

```cpp
auto config = MyPipeline::defaultConfigInfo(WINDOW_WIDTH, WINDOW_HEIGHT);
// 修改 config 的特定字段...
MyPipeline pipeline(device, config, "shader.vert.spv", "shader.frag.spv");
```

## Vulkan 设备绑定

管线需要调用 Vulkan API（创建着色器模块、创建管线等），这些 API 都依赖逻辑设备。因此 `MyPipeline` 持有 `VulkanDevice&` 引用：

```cpp
class MyPipeline {
    private:
        VulkanDevice& device;
        // ...
};
```

构造函数通过成员初始化列表绑定设备：

```cpp
MyPipeline::MyPipeline(
    VulkanDevice& device,
    const PipelineConfigInfo& configInfo,
    const char* vertShaderPath,
    const char* fragShaderPath) : device{device}
```

### 为什么用引用而不是指针？
- 引用语义上表达"必须存在且不可为空"，管线不可能在没有设备的情况下工作
- 引用不可重新绑定，保证管线生命周期内设备不会意外切换

## 着色器模块创建

### VkShaderModule 与 VkPipeline

`MyPipeline` 持有三个 Vulkan 对象句柄：

```cpp
VkPipeline pipeline;              // 管线对象
VkShaderModule vertShaderModule;  // 顶点着色器模块
VkShaderModule fragShaderModule;  // 片段着色器模块
```

它们的关系：

```
VkShaderModule (vert) ──┐
                         ├── VkPipeline (封装全部渲染状态)
VkShaderModule (frag) ──┘
```

- **VkShaderModule**：由 SPIR-V 字节码创建，代表一个已编译的着色器，是管线的着色器阶段的基础
- **VkPipeline**：Vulkan 管线的核心句柄，封装了完整的渲染状态（着色器阶段、顶点输入、光栅化、混合等），后续由 `vkCreateGraphicsPipelines` 创建

### createShaderModule

将 SPIR-V 字节码转换为 `VkShaderModule`：

```cpp
void MyPipeline::createShaderModule(const std::vector<char> bin_code, VkShaderModule *shaderModule)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = bin_code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(bin_code.data());

    if (vkCreateShaderModule(device.device(), &shaderModuleCreateInfo, nullptr, shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module");
    }
}
```

具体流程：

1. 填充 `VkShaderModuleCreateInfo`：
   - `sType`：结构体类型标识，所有 Vulkan 创建信息结构体都需要设置
   - `codeSize`：字节码大小（字节数）
   - `pCode`：指向 SPIR-V 字节码的指针，必须按 `uint32_t` 对齐
2. 调用 `vkCreateShaderModule`，传入逻辑设备，创建着色器模块
3. 创建失败则抛出异常

#### 为什么 `pCode` 需要 `reinterpret_cast<const uint32_t*>` 转换？

因为 SPIR-V 规范要求着色器字节码按 32 位字对齐。`readShaderFile` 返回的是 `std::vector<char>`（逐字节读取），但 Vulkan API 要求以 32 位字为单位解释这些数据，因此需要类型转换。

## 禁止拷贝

```cpp
MyPipeline(const MyPipeline&) = delete;
void operator=(const MyPipeline&) = delete;
```

`MyPipeline` 持有 `VulkanDevice&` 引用和 Vulkan 对象句柄（`VkPipeline`、`VkShaderModule`），拷贝会导致：

- 引用无法重新绑定，拷贝语义不明确
- Vulkan 句柄是独占资源，拷贝后双重销毁会导致未定义行为
