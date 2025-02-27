#pragma once

#include "vulkancontext.h"

namespace SE {

class ShamanEngine
{
public:
    ShamanEngine();
    ~ShamanEngine();
    void run();
    
    std::shared_ptr<VulkanContext> ctx;
    
private:
    
};
}
