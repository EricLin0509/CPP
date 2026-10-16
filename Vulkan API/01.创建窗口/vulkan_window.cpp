#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

static constexpr int WINDOW_WIDTH = 800;
static constexpr int WINDOW_HEIGHT = 600;

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

MyWindow::~MyWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void MyWindow::initWindow()
{
    // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); // 目前 Wayland 需要强制设置为 X11 才能显示窗口

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

bool MyWindow::shouldClose()
{
    return glfwWindowShouldClose(window);
}

int main(void) {
    MyWindow window(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Vulkan!");

    while (!window.shouldClose())
    {
        glfwPollEvents();
    }

    return 0;
}