#pragma once

#include "vulkancontext.h"

namespace SE {

class ShamanEngine
{
public:
    ShamanEngine();
    ~ShamanEngine();
    void run();
    
    VulkanContext* ctx = nullptr;
    
private:
    
};
}
