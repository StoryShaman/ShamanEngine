#include "SeWindow.h"
#include "Config.h"
#include "vulkancontext.h"


namespace SE {
SeWindow::SeWindow(VulkanContext* inctx)
{
    ctx = inctx;
    //std::cout << "SeWindow::SeWindow()" << "\n";
    initWindow();
}

SeWindow::~SeWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool SeWindow::shouldClose()
{
    return glfwWindowShouldClose(ctx->window);
}

void SeWindow::createWindowSurface()
{
    if (glfwCreateWindowSurface(ctx->instance, ctx->window, nullptr, &ctx->surface) != VK_SUCCESS){
        throw std::runtime_error("Failed to create window surface!");
    }
}

void SeWindow::initWindow()
{
    Config& config = Config::get();
    
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    ctx->window = glfwCreateWindow(config.window().width, config.window().height, config.window().name.c_str(), NULL, NULL);
}
}

