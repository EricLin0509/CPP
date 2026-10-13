#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>


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

class MyPipeline {
    private:
        static std::vector<char> readShaderFile(const char* path);

        void createPipeline(const char* vertShaderPath, const char* fragShaderPath);
    public:
        MyPipeline(const char* vertShaderPath, const char* fragShaderPath);
        ~MyPipeline();
};

MyPipeline::MyPipeline(const char* vertShaderPath, const char* fragShaderPath)
{
    createPipeline(vertShaderPath, fragShaderPath);
}

MyPipeline::~MyPipeline(){}


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

void MyPipeline::createPipeline(const char* vertShaderPath, const char* fragShaderPath)
{
    /* 读取着色器文件 */
    auto vertCode = readShaderFile(vertShaderPath);
    auto fragCode = readShaderFile(fragShaderPath);

    std::cout << "Vertex shader code size: " << vertCode.size() << " " << fragCode.size() << "\n";
    std::cout << "Fragment shader code size: " << fragCode.size() << "\n";
}

int main(void) {
    MyWindow window(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Vulkan!");
    MyPipeline pipeline("simple_shader.vert.spv", "simple_shader.frag.spv");

    while (!window.shouldClose())
    {
        glfwPollEvents();
    }

    return 0;
}