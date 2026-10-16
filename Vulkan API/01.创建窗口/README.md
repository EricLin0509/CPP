# 创建窗口

本示例将展示如何使用 Vulkan 创建一个窗口

## 示例

### 引入头文件

需要引入 `GFLW/glfw3.h` 头文件

```cpp
#include <GLFW/glfw3.h>
```

由于我们需要使用 Vulkan API，所以还需要该 `#include` 前定义 `GLFW_INCLUDE_VULKAN`

```cpp
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

### 定义 Window 类

现在我们可以定义一个窗口类 `MyWindow`

里面需要保存四个成员变量：

- 窗口宽度
- 窗口高度
- 窗口标题
- `GLFWwindow` 指针 (**最重要的部分**)

```cpp
class MyWindow {
    private:
        int width;
        int height;
        const char* title;
        GLFWwindow* window;

        void initWindow(); // 用于初始化窗口
    public:
        MyWindow(int width, int height, const char* title);
        MyWindow(const MyWindow&) = delete;
        MyWindow& operator=(const MyWindow&) = delete;
        ~MyWindow();

        bool shouldClose();
};


MyWindow::MyWindow(int width, int height, const char* title)
{
    this->width = width;
    this->height = height;
    this->title = title;
    this->window = nullptr;
    initWindow();
}
```

- 由于创建窗口需要C风格的字符串，所以这里采用 `const char*` 类型

### 初始化窗口

#### 初始化 GLFW 库

在初始化窗口前，需要先初始化 GLFW 库

使用 `glfwInit()` 函数初始化 GLFW 库，该函数签名如下

```cpp
int glfwInit(void);
```

- 返回值：
    - `GLFW_TRUE` 表示初始化成功
    - `GLFW_FALSE` 表示初始化失败

```cpp
void MyWindow::initWindow()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "Failed to initialize GLFW" << "\n";
        return;
    }
}
```

#### 禁用 OpenGL 上下文

由于 GLFW 库默认会创建一个 OpenGL 上下文，需要使用 `glfwWindowHint` 函数禁用 OpenGL 上下文


```cpp
void MyWindow::initWindow()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "Failed to initialize GLFW" << "\n";
        return;
    }

    // 禁用 OpenGL 上下文
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // 禁用窗口可调整大小，暂时
}
```

- 这样后续在创建窗口时，GLFW 就只创建一个裸窗口，不绑定任何图形 API 的上下文
    - 再通过 Vulkan 自己的机制（`VkInstance` + `VkSurfaceKHR`）来与这个窗口关联

#### 创建窗口

使用 `glfwCreateWindow` 函数创建窗口，该函数签名如下

```cpp
GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
```

- 参数：
    - `width`：窗口宽度
    - `height`：窗口高度
    - `title`：窗口标题
    - `monitor`：窗口所属的显示器
    - `share`：与该窗口共享上下文的窗口

- 返回值：
    - 非 `nullptr` 表示创建成功
    - `nullptr` 表示创建失败

```cpp
void MyWindow::initWindow()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "Failed to initialize GLFW" << "\n";
        return;
    }

    // 禁用 OpenGL 上下文
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // 禁用窗口可调整大小，暂时

    // 创建窗口
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create window" << "\n";
        return;
    }
}
```

### 应用主循环

#### 检查窗口是否需要关闭

使用 `glfwWindowShouldClose` 函数检查窗口是否需要关闭，该函数签名如下

```cpp
int glfwWindowShouldClose(GLFWwindow* window);
```

- 参数：
    - `window`：窗口指针

- 返回值：
    - `GLFW_TRUE` 表示窗口需要关闭
    - `GLFW_FALSE` 表示窗口不需要关闭

```cpp
bool MyWindow::shouldClose()
{
    return glfwWindowShouldClose(window);
}
```

```cpp
MyWindow window(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Vulkan!");
while (!window.shouldClose());
```

#### 处理窗口事件

使用 `glfwPollEvents` 函数轮询窗口事件，该函数签名如下

```cpp
void glfwPollEvents(void);
```

```cpp
while (!window.shouldClose())
{
    glfwPollEvents();
}
```

### 销毁实例

#### 销毁窗口

使用 `glfwDestroyWindow` 函数销毁窗口

```cpp
MyWindow::~MyWindow()
{
    glfwDestroyWindow(window);
}
```

#### 最终清理

使用 `glfwTerminate` 函数最终清理

```cpp
void MyWindow::~MyWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
```

### 编译

需要添加 `pkg-config --static --libs glfw3` 编译选项

```bash
g++ -o vulkan_window vulkan_window.cpp `pkg-config --static --libs glfw3`
```
