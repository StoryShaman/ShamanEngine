#pragma once

#define GLFW_INCLUDE_VULKAN
#include <memory>
#include <GLFW/glfw3.h>



namespace SE {
struct VulkanContext;

class SeWindow
{
public:
    SeWindow(std::shared_ptr<VulkanContext> inctx);
    ~SeWindow();
    bool shouldClose();
    void createWindowSurface();

public:
    GLFWwindow* window;
    VkSurfaceKHR surface;

    int width;
    int height;
    bool framebufferResized = false;
    
private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    
    void initWindow();
    std::shared_ptr<VulkanContext> ctx;
    
};

}
