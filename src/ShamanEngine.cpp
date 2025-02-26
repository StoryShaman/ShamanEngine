#include "ShamanEngine.h"

#include <iostream>
#include <ostream>
#include <GLFW/glfw3.h>

#include "SePipeline.h"
#include "SeRenderer.h"
#include "SeWindow.h"
#include "vulkancontext.h"

namespace SE {
ShamanEngine::ShamanEngine()
{
    ctx = new VulkanContext{};
    ctx->Se_window = new SeWindow(ctx);
    ctx->Se_device = new SeDevice(ctx);
    ctx->Se_swapchain = new SeSwapChain(ctx);
    ctx->Se_renderer = new SeRenderer(ctx);
    
    

    std::cout << ctx << std::endl;
    
}

ShamanEngine::~ShamanEngine()
{
}

void ShamanEngine::run()
{
    while (!ctx->Se_window->shouldClose())
    {
        glfwPollEvents();
    }
}

}
