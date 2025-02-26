#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace SE {
struct VulkanContext;

class SeWindow
{
public:
    SeWindow(VulkanContext* inctx);
    ~SeWindow();
    bool shouldClose();
    void createWindowSurface();
    
    GLFWwindow* window;
private:
    
    void initWindow();
    VulkanContext* ctx;
    
};

}
