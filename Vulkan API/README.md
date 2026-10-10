# Vulkan API

Vulkan API 是一个跨平台的 3D 图形与计算 API，由 Khronos Group 制定和维护。它提供了一种高度并行的渲染方式，能够充分利用 GPU 的所有性能。

## 核心特性

- **显式控制**：Vulkan 几乎不提供默认行为，开发者需要对 GPU 资源分配、同步、内存管理等拥有完全控制权
- **跨平台**：支持 Windows、Linux、Android、macOS（通过 MoltenVK）等平台
- **多线程友好**：命令缓冲区的构建可以在多个线程中并行执行，充分利用多核 CPU
- **低开销**：减少了驱动程序的运行时验证和隐式同步，运行时开销极低
- **计算着色器**：原生支持通用计算（GPGPU），不仅限于图形渲染
- **SPIR-V**：使用中间着色语言 SPIR-V，着色器在编译时即被转换为优化的中间表示

## 与 OpenGL 的对比

| 特性       | OpenGL              | Vulkan                |
| :--------: | :-----------------: | :-------------------: |
| 控制级别   | 隐式/高层           | 显式/底层             |
| 驱动开销   | 较高（隐式验证）    | 极低（验证可选）     |
| 多线程     | 上下文单线程限制    | 原生多线程支持       |
| 着色器     | GLSL（运行时编译）  | SPIR-V（离线编译）   |
| 错误检测   | 驱动隐式处理        | 需显式启用验证层     |
| 学习曲线   | 相对平缓            | 陡峭                  |

## 核心概念

- **Instance**：Vulkan 应用程序与驱动的连接入口
- **Physical Device**：系统中可用的 GPU 硬件
- **Logical Device**：与物理设备交互的逻辑接口
- **Queue**：提交命令的执行队列（图形、计算、传输等）
- **Swapchain**：管理呈现图像的链，连接渲染结果与窗口系统
- **Render Pass**：描述渲染过程中的附件和子pass的依赖关系
- **Pipeline**：描述完整的渲染或计算流水线状态（着色器、光栅化、混合等）
- **Command Buffer**：记录 GPU 命令的缓冲区，提交到队列执行
- **Descriptor**：着色器与资源（uniform buffer、纹理等）的绑定接口
- **Validation Layers**：可选的调试层，用于在开发期间检测错误和性能问题

## 依赖项

- **Vulkan SDK**：包含头文件、加载器、验证层和工具（推荐使用 LunarG Vulkan SDK）
- **GLFW / SDL2**：窗口创建与输入管理
- **GLM**：数学库（向量、矩阵运算，与 GLSL 兼容）
- **stb_image**：图像加载库（纹理加载）

## 学习资源

- [Vulkan 官方规范](https://www.khronos.org/registry/vulkan/specs/1.3/html/)
- [Vulkan Tutorial](https://vulkan-tutorial.com/) — 经典入门教程
- [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home) — 官方 SDK 下载
- [Sascha Willems 的 Vulkan 示例](https://github.com/SaschaWillems/Vulkan) — 丰富的代码示例
