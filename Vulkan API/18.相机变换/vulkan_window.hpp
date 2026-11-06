#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace lve {
class VulkanWindow {
    private:
        int width;
        int height;
        bool framebufferResized;
        const char* title;
        GLFWwindow* window;

        void initWindow(); // 用于初始化窗口
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    public:
        VulkanWindow(int width, int height, const char* title);
        VulkanWindow(const VulkanWindow&) = delete;
        VulkanWindow& operator=(const VulkanWindow&) = delete;
        ~VulkanWindow();
        
        bool shouldClose();

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

        bool isFramebufferResized() { return framebufferResized; }
        void resetFramebufferResized() { framebufferResized = false; }
};
};