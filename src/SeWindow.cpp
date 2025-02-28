#include "SeWindow.h"
#include "Config.h"
#include "vulkancontext.h"


namespace SE {
SeWindow::SeWindow(std::shared_ptr<VulkanContext> inctx)
{
    ctx = inctx;
    initWindow();
    createWindowSurface();
    glfwGetWindowSize(window, &width, &height);
}

SeWindow::~SeWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void SeWindow::initWindow()
{
    Config& config = Config::get();
    
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(
        static_cast<uint32_t>(config.window().width),
        static_cast<uint32_t>(config.window().height),
        config.window().name.c_str(),
        NULL,
        NULL
        );

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

bool SeWindow::shouldClose()
{
    return glfwWindowShouldClose(window);
}

void SeWindow::createWindowSurface()
{
    VkResult result = glfwCreateWindowSurface(ctx->Se_engine->instance, window, nullptr, &surface);
    if (result != VK_SUCCESS){
        std::cerr << "glfwCreateWindowSurface"  << result;
        throw std::runtime_error("Failed to create window surface!");
    }
}


void SeWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto SeWindow = reinterpret_cast<SE::SeWindow*>(glfwGetWindowUserPointer(window));
    SeWindow->framebufferResized = true;
    SeWindow->width = width;
    SeWindow->height = height;
}


}

