# 设备初始化

本示例将展示如何使用 Vulkan 初始化设备，包括创建 Instance、设置验证层、选择物理设备、创建逻辑设备等

## 示例

### 引入头文件

需要引入 Vulkan 设备相关的头文件

```cpp
#include "my_engine_device.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>
```

- `cstring`：用于 `strcmp` 比较验证层名称
- `set`：用于去重队列族索引
- `unordered_set`：用于高效查找可用扩展

### 定义 VulkanDevice 类

VulkanDevice 类封装了 Vulkan 设备的完整初始化流程，包含以下核心成员变量：

- `instance`：Vulkan 实例，是与 Vulkan API 交互的入口
- `debugMessenger`：调试信使，用于接收验证层消息
- `physicalDevice`：物理设备（GPU），代表实际的硬件
- `device_`：逻辑设备，是程序与 GPU 交互的接口
- `surface_`：窗口表面，用于将渲染结果呈现到窗口
- `graphicsQueue_`：图形队列，用于提交图形命令
- `presentQueue_`：呈现队列，用于提交呈现命令
- `commandPool`：命令池，用于分配命令缓冲区

```cpp
class VulkanDevice {
 public:
  VulkanDevice(VulkanWindow &window);
  ~VulkanDevice();

  // Not copyable or movable
  VulkanDevice(const VulkanDevice &) = delete;
  void operator=(const VulkanDevice &) = delete;
  VulkanDevice(VulkanDevice &&) = delete;
  VulkanDevice &operator=(VulkanDevice &&) = delete;

 private:
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // `VK_NULL_HANDLE` 表示未选择物理设备
  VulkanWindow &window;
  VkCommandPool commandPool;

  VkDevice device_;
  VkSurfaceKHR surface_;
  VkQueue graphicsQueue_;
  VkQueue presentQueue_;
};
```

它的结构大致如下

```
VkInstance
  ├── VkDebugUtilsMessengerEXT
  ├── VkSurfaceKHR
  └── VkPhysicalDevice
        └── VkDevice (Logical)
              ├── VkQueue (Graphics / Present)
              ├── VkCommandPool
              ├── VkBuffer + VkDeviceMemory
              └── VkImage + VkDeviceMemory
```

- VulkanDevice 不可拷贝、不可移动，因为 Vulkan 的内部对象是唯一的资源句柄

### 构造函数：初始化流程

VulkanDevice 的构造函数按顺序完成所有初始化步骤

1. 创建 Vulkan Instance
	- 它是与 Vulkan API 交互的入口，是所有 Vulkan 对象的父对象
2. 设置调试信使
	- 用于接收验证层消息，帮助调试
	- 开启后会增加性能开销，推荐只在调试环境下开启
3. 创建窗口表面
	- 用于将渲染结果呈现到窗口，是 Vulkan 与窗口系统交互的桥梁
4. 选择物理设备
	- 评估可用的物理设备（GPU），选择最符合要求的设备，可以同时选择多个物理设备（不过十分复杂）
5. 创建逻辑设备
	- 选择物理设备后，创建逻辑设备，这是程序与 GPU 交互的接口，类似 HAL 硬件抽象层
	- 可以类比为软件与硬件之间一个特殊的插座
6. 创建命令池
	- 用于分配命令缓冲区，是提交命令到 GPU 的方式

这个初始化顺序很重要：

- Instance **必须**最先创建，因为 Surface 需要依赖 Instance
- Physical Device 需要依赖 Surface 评估队列族
- Logical Device 需要依赖 Physical Device

```cpp
VulkanDevice::VulkanDevice(VulkanWindow &window) : window{window} {
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createCommandPool();
}
```

### 析构函数：销毁流程

销毁顺序与创建顺序**相反**（Vulkan 的标准做法）

```cpp
VulkanDevice::~VulkanDevice() {
  vkDestroyCommandPool(device_, commandPool, nullptr);
  vkDestroyDevice(device_, nullptr);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(instance, surface_, nullptr);
  vkDestroyInstance(instance, nullptr);
}
```

- 先销毁 Command Pool 和 Logical Device（子对象）
- 再销毁 Debug Messenger、Surface 和 Instance（父对象）
- 如果使用错误的销毁顺序在 Vulkan 文档中属于未定义行为
	- 可能会导致段错误、GPU驱动崩溃或GPU死锁

#### 错误的销毁顺序具体后果

- 段错误 / 崩溃：如果父对象（如 `VkInstance`）在子对象（如 `VkDevice`）仍处于活跃或被引用状态时被销毁，驱动程序可能会尝试访问已释放的内部内存映射，从而导致应用程序立即崩溃。
- 验证层错误：Vulkan 验证层通常会标记资源泄漏相关的警告或错误——例如，当 `VkBuffer` 或 `VkImage` 在销毁顺序不当时，仍持有对 `VkDeviceMemory` 对象的有效引用。
- 资源泄漏 / 死锁：过早销毁对象，或者在清理前未通过 `vkDeviceWaitIdle(device)` 等待 GPU 完成工作，可能会导致驱动程序侧的句柄或内存池无法释放。
- Surface 与 Swapchain 不匹配：在逻辑设备或 Swapchain 之前销毁 WSI Surface，可能会触发 Surface 丢失错误或特定于平台的驱动程序故障。

例如 `vkDestroyDevice` 放在 `vkDestroyCommandPool` 之前

```cpp
vkDestroyDevice(device_, nullptr);
vkDestroyCommandPool(device_, commandPool, nullptr);
```

它会这样报错

```bash
validation layer: vkDestroyDevice(): VkDevice 0x56df0b0d37d0 has 1 leaked objects that have not been destroyed.
VkCommandPool 0x30000000003
The Vulkan spec states: All child objects created on device that can be destroyed or freed must have been destroyed or freed prior to destroying device (https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#VUID-vkDestroyDevice-device-05137)
[Vulkan Loader] ERROR:          vkDestroyCommandPool: Invalid device [VUID-vkDestroyCommandPool-device-parameter]
[1]    15756 abort (core dumped)  ./main
```

## 创建 Vulkan Instance

### 检查验证层支持

在创建 Instance 之前，如果启用了验证层，需要先检查系统是否支持所需的验证层

验证层 (Validation Layer) 是一种可选的 Vulkan 扩展。由于 Vulkan 旨在提供极低的驱动开销和极高的底层控制，驱动程序默认几乎不进行任何错误检查（不检查参数合法性、不防范多线程冲突）。校验层正是用来在开发阶段补充这些检查的机制

```cpp
bool VulkanDevice::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}
```

- 使用 `vkEnumerateInstanceLayerProperties` 获取所有可用的验证层
- 逐一检查所需的验证层（`VK_LAYER_KHRONOS_validation`）是否在可用列表中

### 获取所需扩展

```cpp
std::vector<const char *> VulkanDevice::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}
```

- `glfwGetRequiredInstanceExtensions`：获取 GLFW 所需的 Vulkan 扩展（用于创建 Surface）
- 如果启用验证层，还需要添加 `VK_EXT_DEBUG_UTILS_EXTENSION_NAME` 扩展

### 创建 Instance

使用 `vkCreateInstance` 函数创建 Vulkan Instance，该函数签名如下

```cpp
VkResult vkCreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance);
```

- 参数：
    - `pCreateInfo`：创建信息结构体指针
    - `pAllocator`：自定义内存分配器（通常为 `nullptr`）
    - `pInstance`：输出创建的 Instance

- 返回值：
    - `VK_SUCCESS` 表示创建成功

```cpp
void VulkanDevice::createInstance() {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "LittleVulkanEngine App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }

  hasGflwRequiredInstanceExtensions();
}
```

- `VkApplicationInfo`：提供应用和引擎信息，不影响程序运行，但驱动可以根据此信息优化
- `VkInstanceCreateInfo`：Instance 创建的核心配置
    - 设置启用的扩展和验证层
    - 如果启用验证层，通过 `pNext` 传递 `debugCreateInfo` 以便在 Instance 创建期间就能捕获错误

## 设置调试信使

### 调试回调函数

调试信使通过回调函数接收验证层消息

```cpp
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}
```

- `messageSeverity`：消息严重程度（Verbose、Info、Warning、Error）
- `messageType`：消息类型（General、Validation、Performance）
- `pCallbackData`：包含消息详细信息的结构体
- 返回值：
    - `VK_TRUE` 表示触发中断
    - `VK_FALSE` 表示继续正常执行

### 填充调试信使创建信息

```cpp
void VulkanDevice::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;
}
```

- 只接收 Warning 和 Error 级别的消息（过滤掉 Verbose 和 Info）
- 接收所有类型的消息（General、Validation、Performance）

### 创建和销毁调试信使

由于 `vkCreateDebugUtilsMessengerEXT` 和 `vkDestroyDebugUtilsMessengerEXT` 是扩展函数，需要通过 `vkGetInstanceProcAddr` 动态加载

```cpp
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance,
      "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}
```

- `vkGetInstanceProcAddr`：根据函数名获取函数指针
- 如果函数不存在（扩展未加载），返回 `VK_ERROR_EXTENSION_NOT_PRESENT`

```cpp
void VulkanDevice::setupDebugMessenger() {
  if (!enableValidationLayers) return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);
  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}
```

## 创建窗口表面

```cpp
void VulkanDevice::createSurface() { window.createWindowSurface(instance, &surface_); }
```

- Surface 是 Vulkan 与窗口系统之间的桥梁，将渲染结果呈现到窗口上
- 委托给 `VulkanWindow` 类创建，因为 Surface 的创建与窗口系统（GLFW）紧密相关

## 选择物理设备

### 枚举物理设备

使用 `vkEnumeratePhysicalDevices` 获取所有支持 Vulkan 的 GPU

```cpp
void VulkanDevice::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::cout << "Device count: " << deviceCount << std::endl;
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto &device : devices) {
    if (isDeviceSuitable(device)) {
      physicalDevice = device;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  std::cout << "physical device: " << properties.deviceName << std::endl;
}
```

- 首先查询设备数量，然后分配空间并获取所有设备
- 遍历设备，选择第一个满足要求的设备
- 使用 `vkGetPhysicalDeviceProperties` 获取设备属性（如设备名称）

### 评估设备是否合适

```cpp
bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}
```

- 设备必须满足以下所有条件：
    - 支持所需的队列族（图形队列 + 呈现队列）
    - 支持所需的设备扩展（`VK_KHR_swapchain`）
    - Swap Chain 有足够的格式和呈现模式
    - 支持各向异性采样（`samplerAnisotropy`）

### 查找队列族

```cpp
QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.graphicsFamilyHasValue = true;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilyHasValue = true;
    }
    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}
```

- `QueueFamilyIndices` 结构体记录图形队列族和呈现队列族的索引
    - 图形队列族：支持 `VK_QUEUE_GRAPHICS_BIT` 标志
    - 呈现队列族：通过 `vkGetPhysicalDeviceSurfaceSupportKHR` 检查是否支持向 Surface 呈现
- 两个队列族可能是同一个（在大多数 GPU 上是这样），但也可能不同

#### 为什么需要图形队列和呈现队列？

##### 它们各自的职责

- **图形队列（Graphics Queue）**：负责执行**渲染命令**——顶点处理、光栅化、着色器执行等。它对应 `VK_QUEUE_GRAPHICS_BIT` 标志
- **呈现队列（Present Queue）**：负责将渲染好的图像**提交（呈现）到屏幕**上。它通过 `vkGetPhysicalDeviceSurfaceSupportKHR` 检查是否支持向特定 Surface 呈现

##### 为什么不合并成一个？

这是 Vulkan 设计上的一个重要抽象，原因如下：

1. **硬件架构差异**：在某些 GPU（尤其是嵌入式/移动端）上，图形处理和画面呈现可能由**不同的硬件单元**完成。图形队列由 GPU 的渲染核心处理，而呈现队列可能由显示控制器（Display Controller）处理

2. **多 GPU 环境**：在多 GPU 系统中，可能一个 GPU 负责渲染，另一个 GPU 负责显示输出。这种情况下图形队列和呈现队列必然在不同的物理设备上

3. **灵活性**：Vulkan 的设计哲学是**不隐藏硬件细节**，让开发者能够精确控制。将两者分开，使得应用可以：
  - 在一个 GPU 上渲染，在另一个 GPU 上呈现
  - 使用不同的队列族来并行执行渲染和呈现
  - 适配各种硬件拓扑结构

简单来说，图形队列是负责计算与渲染 (不同于纯计算队列 `VK_QUEUE_COMPUTE_BIT`)，呈现队列是负责显示与同步

##### 实际情况

在代码中可以看到：

```cpp
std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};
```

这里用 `std::set` 去重，因为**在大多数桌面 GPU 上，图形队列族和呈现队列族实际上是同一个队列族**（索引相同）。但 Vulkan 仍然要求我们分别查询，因为规范**不保证**它们一定相同

##### 类比

可以把它们想象成工厂和物流：

| | 图形队列 | 呈现队列 |
| :---: | :---: | :---: |
| **比喻** | 工厂生产产品 | 物流将产品送到商店 |
| **操作** | 执行渲染管线 | 将 SwapChain 图像呈现到屏幕 |
| **硬件** | GPU 渲染核心 | 显示控制器 |

大多数情况下工厂和物流在同一个公司（同一个队列族），但 Vulkan 不做这个假设，以保持最大的硬件兼容性

### 检查设备扩展支持

由于每个 Vulkan 版本必须支持的设备扩展不同，需要先检查是否支持这些扩展

简单的说就是检查该GPU是否支持我们需要的的特定硬件功能

```cpp
bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}
```

- 将所需扩展放入 `std::set` (插入/删除为 `O(logn)` 时间复杂度)，然后逐一删除已找到的扩展
- 如果最终集合为空，说明所有所需扩展都已支持

## 创建逻辑设备

逻辑设备是程序与 GPU 交互的接口，需要指定要启用的队列和扩展

```cpp
void VulkanDevice::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
  vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
}
```

- 使用 `std::set` 对队列族去重，避免为同一个队列族创建多个队列
- 队列优先级范围为 `[0.0, 1.0]`，默认为 `1.0`（最高优先级）
- 启用 `samplerAnisotropy` 特性（各向异性过滤）
	- 这用于改善倾斜纹理的渲染质量，避免锯齿状边缘
- 创建逻辑设备后，使用 `vkGetDeviceQueue` 获取队列句柄

### 为什么要这样大费周章？（传统 API 的血泪教训）

在没有彻底分离 HAL 的老一代 API（比如 OpenGL）中，程序直接和一个大而全的“上下文（Context）”绑定

- 痛点：驱动程序在底层默默地做了一堆脏活累活。如果换了一块显卡，或者驱动更新了，底层代码的行为可能会发生不可预测的变化
- Vulkan 的解决办法：通过 `VkPhysicalDevice` 让硬件细节暴露在阳光下，让开发者在运行时动态检查；然后通过 `VkDevice`（HAL 层）把我们确切想要的功能硬编码确定下来。这样一来，无论底层的物理显卡怎么变，我们在软件端面对的逻辑设备接口都是稳定、一致且完全受控的

## 创建命令池

命令池（Command Pool）是用来分配和管理命令缓冲区（Command Buffers）的内存池

简单来说，Vulkan 的所有渲染和计算指令（比如画一个三角形、绑定一个着色器）都必须先录制到“命令缓冲区”里，然后一起提交给 GPU 执行。而命令池就是这些缓冲区的后台内存管理者

```cpp
void VulkanDevice::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
  poolInfo.flags =
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}
```

- `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT`：提示命令缓冲区是短生命周期的（频繁重新录制）
- `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`：允许单独重置命令缓冲区

## 辅助功能

### 查找支持的格式

用来获取某种特定图像格式（VkFormat）在当前显卡上的硬件支持细节

```cpp
VkFormat VulkanDevice::findSupportedFormat(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (
        tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}
```

- `VkFormatProperties`: 是一个结构体，包含以下数据
  - `linearTilingFeatures`：当图像在内存中以线性平铺（Linear Tiling）方式排列时，硬件支持的特性
  - `optimalTilingFeatures`：当图像在内存中以最优平铺（Optimal Tiling）方式排列时（GPU 硬件专属优化格式，无法由 CPU 直接读写），硬件支持的特性
  - `bufferFeatures`：如果把这个格式当作缓冲区（Buffer）的格式（例如 Uniform Texel Buffer）时，硬件支持的特性

- `VkFormatFeatureFlags` 是一个枚举类型，有以下**位掩码**
  - `VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT`：该格式是否可以作为深度/模板缓冲区使用（渲染深度图时必须检查此项）
  - `VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT`：该格式是否能在 Shader 中被 Sampled（采样）（普通纹理贴图必须检查此项）
  - `VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT`：该格式是否能作为颜色渲染附着（即可以直接渲染画面到上面）

- 按优先级遍历候选格式，返回第一个满足 tiling 和 features 要求的格式
- 用于查找深度缓冲等所需的格式

通过 `vkGetPhysicalDeviceFormatProperties`，显卡驱动会极其诚实地告诉你：“我这块显卡支持 `D32_SFLOAT` 格式作为深度图，但并不支持它作为普通纹理进行线性采样”

这可以在初始化阶段就能规划好最佳的渲染管线，避免在运行时发生意料之外的硬件不匹配或性能暴跌

### 查找内存类型

这用于当前电脑上那块GPU的内存架构到底是怎样的

它到底有多少种不同类型的内存类型（Memory Types）以及多少个内存堆（Memory Heaps）可用

```cpp
uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}
```

- `VkPhysicalDeviceMemoryProperties`: 结构体，包含以下数据
  - `memoryTypeCount`：内存类型数量
  - `memoryTypes`：内存类型数组，每个元素表示一种内存类型
  - `memoryHeapCount`：内存堆数量
  - `memoryHeaps`：内存堆数组，每个元素表示一种内存堆

- `VkMemoryPropertyFlags` 是一个枚举类型，有以下**位掩码**
  -  `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`
	  - 含义：GPU 本地显存
	  - 特点：GPU 访问极快，但 CPU 无法直接读写（必须通过暂存缓冲区 Staging Buffer 靠 GPU 内部传输过去）
	  - 用途：放置不需要每帧都修改的高频资源，如顶点缓冲区（Vertex Buffer）、纹理图、深度缓冲区
  - `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`
	  - 含义：CPU 可见内存
	  - 特点：CPU 可以直接通过 vkMapMemory 映射并写入数据
	  - 用途：用于将数据从 CPU 传输到 GPU。通常作为暂存缓冲区（Staging Buffer）或者频繁更新的常量缓冲区（Uniform Buffer）
  - `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`
	  - 含义：CPU/GPU 内存一致性
	  - 特点：当 CPU 写入数据后，不需要手动调用 `vkFlushMappedMemoryRanges` 刷新缓存，数据会自动对 GPU 可见
	  - 用途：通常与 `HOST_VISIBLE` 绑定使用

- `typeFilter`：位掩码，表示哪些内存类型是可行的（由 `vkGetBufferMemoryRequirements` 返回）
- 逐一检查每个内存类型是否同时满足位掩码和属性要求

#### 内存堆去哪里了？

这里没有用上 `memoryHeaps` 成员是因为在 `VkMemoryType` 结构体 (即 `memoryTypes` 成员类型) 是这样定义的

```c
typedef struct VkMemoryType {
    VkMemoryPropertyFlags    propertyFlags; // 内存属性标志（如 HOST_VISIBLE）
    uint32_t                 heapIndex;     // 它所属的内存堆索引
} VkMemoryType;
```

当调用 `findMemoryType` 并最终把返回的 `memoryTypeIndex` 传给 `vkAllocateMemory` 时，驱动程序只要一看到这个类型索引，就能顺藤摸瓜直接找到它归属的内存堆，并从该堆的物理容量里扣除相应的内存

因此，不需要也没有必要在函数里手动去指定或匹配内存堆

除非要**做内存预算和防崩溃检查**或**自定义高性能内存分配器**才需要调用该成员

#### 内存类型与内存堆的区别

| 特征 | 内存堆 (VkMemoryHeap) | 内存类型 (VkMemoryType) |
| :--: | :------------------: | :-------------------: |
| 本质概念 | 物理硬件的分类和大小总量 |	物理内存之上的驱动逻辑封装 |
| 决定了什么 | 资源的物理位置（在显存里还是内存里） |	资源的访问方式（CPU能不能读写、需不需要刷缓存） |
| 核心数据 | 内存的字节大小（size） |	对应的属性标志（`propertyFlags`） |
| 数量关系 | 数量较少（PC 上通常只有 2 ~ 3 个） |	数量较多（通常有 10 ~ 32 种 类型，分别归属于不同的堆） |

### 创建缓冲区

由于 Vulkan 不允许**直接把C++的内存指针丢给显卡**

所以必须通过创建缓冲区，在显存或系统内存里开辟出专属的存储空间，把数据整整齐齐地码在里面，供 GPU 读取

```cpp
void VulkanDevice::createBuffer(
    VkDeviceSize size,                // 1. 你要装多少字节的数据？
    VkBufferUsageFlags usage,         // 2. 这是干嘛用的？（顶点？索引？还是暂存？）
    VkMemoryPropertyFlags properties, // 3. 空间开在哪里？（CPU可见还是GPU专属？）
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(device_, buffer, bufferMemory, 0);
}
```

- 创建缓冲区的三步流程：
  1. `vkCreateBuffer`：创建缓冲区对象
  2. `vkAllocateMemory`：根据缓冲区的内存需求分配设备内存
  3. `vkBindBufferMemory`：将缓冲区与分配的内存绑定

总的来说，所有 Vulkan 资源的生命周期都可以总结为四个步骤：

1. 创建对象
2. 分配内存
3. 绑定内存
4. 销毁对象

### 单次命令提交

用于执行不需要持久化的临时命令（如缓冲区复制、图像布局转换）

#### 渲染循环命令 vs 单次命令

我们可以把 Vulkan 中的命令缓冲区（Command Buffers）分为两类：

- 渲染循环命令（每帧都在跑）： 比如绘制 3D 场景、清除屏幕、切换帧缓冲。这些命令在游戏运行期间，每秒钟要被 GPU 反复执行几十甚至上百次。它们通常在初始化时录制好，或者每帧重复使用
- 单次命令（用完即弃）： 比如在游戏加载时，把 3D 模型的顶点数据从内存复制到显存（copyBuffer），或者把一张 PNG 纹理图片上传到 GPU（`copyBufferToImage`）。这些操作只在资源初始化时发生一次，后面就再也不需要执行了

```cpp
VkCommandBuffer VulkanDevice::beginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue_);

  vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}
```

- `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`：提示驱动此命令缓冲区只提交一次，可优化执行
- 提交后使用 `vkQueueWaitIdle` 等待队列空闲，确保命令执行完毕
- 最后释放命令缓冲区

### 复制缓冲区

```cpp
void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer);
}
```

- 使用单次命令提交机制，通过 `vkCmdCopyBuffer` 在 GPU 端执行缓冲区复制

### 复制缓冲区到图像

```cpp
void VulkanDevice::copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layerCount;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(
      commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  endSingleTimeCommands(commandBuffer);
}
```

- 用于将暂存缓冲区（staging buffer）中的像素数据复制到图像对象
- 图像必须处于 `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL` 布局（传输目标布局）

### 创建图像

```cpp
void VulkanDevice::createImageWithInfo(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory) {
  if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device_, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
    throw std::runtime_error("failed to bind image memory!");
  }
}
```

- 与 `createBuffer` 类似的三步流程：创建图像 → 分配内存 → 绑定内存
- 通过 `VkImageCreateInfo` 参数可以灵活配置图像的各种属性（尺寸、格式、用途等）
