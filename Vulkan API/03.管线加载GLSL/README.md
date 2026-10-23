# 管线加载 GLSL 着色器

本示例将展示如何编写 GLSL 着色器、编译着色器为 SPIR-V、并在 C++ 中读取着色器代码

## 示例

### 编写着色器

#### 顶点着色器

顶点着色器负责将顶点位置转换为齐次坐标

```glsl
#version 450

vec2 positions[3] = vec2[] (
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
```
- `#version 450`：指定 GLSL 版本为 4.5，对应 Vulkan 1.0
- `positions[3]`：定义三角形的三个顶点位置
    - `(0.0, -0.5)`：顶部顶点
    - `(0.5, 0.5)`：右下顶点
    - `(-0.5, 0.5)`：左下顶点
- `gl_VertexIndex`：Vulkan 提供的内置变量，表示当前顶点的索引
- `gl_Position`：内置输出变量，顶点的齐次坐标 (x, y, z, w)
    - `vec4(positions[gl_VertexIndex], 0.0, 1.0)`：将二维坐标扩展为齐次坐标，z=0.0（在近裁剪面上），w=1.0

#### 片段着色器

片段着色器负责输出每个像素的颜色

```glsl
#version 450

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
```
- `layout(location = 0) out vec4 fragColor`：声明输出变量，`location = 0` 指定输出位置对应渲染目标的第 0 个附件
- `out`: 表示这是一个输出变量，自己处理完之后，准备传给下一个阶段
- `in`: 表示这是一个输入变量，接收上一个阶段的输出
- `vec4(1.0, 0.0, 0.0, 1.0)`：RGBA 颜色值，表示**红色**（R=1.0, G=0.0, B=0.0, A=1.0）
- 因此最终渲染结果为一个**红色三角形**

### 编译着色器
Vulkan 使用 SPIR-V 字节码格式，需要使用 `glslc` 编译器将 GLSL 着色器编译为 SPIR-V

```bash
glslc -o simple_shader.vert.spv simple_shader.vert
glslc -o simple_shader.frag.spv simple_shader.frag
```

- `glslc`：Khronos 提供的 GLSL 到 SPIR-V 编译器
- `-o`：指定输出文件名
- `.spv`：SPIR-V 字节码文件的标准扩展名
- 编译后生成 `simple_shader.vert.spv` 和 `simple_shader.frag.spv` 两个文件，供 C++ 程序读取

### 读取着色器文件

#### 定义 MyPipeline 类

渲染管线类 `MyPipeline` 负责读取着色器文件并创建渲染管线

```cpp
class MyPipeline {
    private:
        static std::vector<char> readShaderFile(const char* path);

        void createPipeline(const char* vertShaderPath, const char* fragShaderPath);
    public:
        MyPipeline(const char* vertShaderPath, const char* fragShaderPath);
        ~MyPipeline();
};
```
- `readShaderFile`：静态方法，读取着色器 SPIR-V 文件到字节缓冲区
- `createPipeline`：创建渲染管线（本示例中仅读取着色器代码并打印大小）

#### 读取着色器文件到缓冲区

使用 `std::ifstream` 以二进制模式读取着色器文件

```cpp
std::vector<char> MyPipeline::readShaderFile(const char* path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}
```
- `std::ios::ate`：打开文件时定位到文件末尾，用于获取文件大小
- `std::ios::binary`：以二进制模式读取，避免文本模式下的换行符转换
- `file.tellg()`：获取当前文件指针位置（即文件大小）
- `file.seekg(0)`：将文件指针重置到文件开头，准备读取全部内容
- 返回 `std::vector<char>`：以字节缓冲区形式返回着色器代码

#### 创建渲染管线
```cpp
void MyPipeline::createPipeline(const char* vertShaderPath, const char* fragShaderPath)
{
    /* 读取着色器文件 */
    auto vertCode = readShaderFile(vertShaderPath);
    auto fragCode = readShaderFile(fragShaderPath);

    std::cout << "Vertex shader code size: " << vertCode.size() << " " << fragCode.size() << "\n";
    std::cout << "Fragment shader code size: " << fragCode.size() << "\n";
}
```
- 本示例中仅读取着色器代码并打印大小，后续示例将逐步完善管线的创建

### 编译
需要添加 `pkg-config --static --libs glfw3` 编译选项
```bash
g++ -o vulkan_window vulkan_window.cpp `pkg-config --static --libs glfw3` -lvulkan
```
