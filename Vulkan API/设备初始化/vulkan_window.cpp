#include "vulkan_window.hpp"

#include <iostream>

using namespace lve;

VulkanWindow::VulkanWindow(int width, int height, const char* title)
{
    this->width = width;
    this->height = height;
    this->title = title;
    this->window = nullptr;
    initWindow();
}

VulkanWindow::~VulkanWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanWindow::initWindow()
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

bool VulkanWindow::shouldClose()
{
    return glfwWindowShouldClose(window);
}

void VulkanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}