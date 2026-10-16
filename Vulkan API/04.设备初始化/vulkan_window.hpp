#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace lve {
class VulkanWindow {
    private:
        int width;
        int height;
        const char* title;
        GLFWwindow* window;

        void initWindow(); // 用于初始化窗口
    public:
        VulkanWindow(int width, int height, const char* title);
        VulkanWindow(const VulkanWindow&) = delete;
        VulkanWindow& operator=(const VulkanWindow&) = delete;
        ~VulkanWindow();
        
        bool shouldClose();

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
};
};