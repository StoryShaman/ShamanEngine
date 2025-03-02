#pragma once
#include <memory>

#include "ShamanEngine.h"
#include "SeModel.h"
#include "SeSwapChain.h"
#include "SeWindow.h"
#include "SePipeline.h"


namespace SE {
class SeCamera;
class SeSwapChain;
class SeRenderer;
class SeWindow;
class SeDevice;
class SePipeline;




struct VulkanContext
{
    ShamanEngine* Se_engine = nullptr;
    SeWindow* Se_window = nullptr;
    SeDevice* Se_device = nullptr;
    SePipeline* Se_pipeline = nullptr;
    SeSwapChain* Se_swapchain = nullptr;
    SeRenderer* Se_renderer = nullptr;
    SeCamera* Se_camera = nullptr;
    std::shared_ptr<SeModel> Se_model = nullptr;

    
    
};
    
}
