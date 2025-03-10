﻿#include "ShamanEngine.h"

#include <chrono>
#include <iostream>
#include <ostream>
#include <unordered_set>


#include "Config.h"
#include "SeController.h"
#include "SeDevice.h"
#include "SeObject.h"
#include "SePipeline.h"
#include "SeRenderer.h"
#include "SeWindow.h"
#include "vulkancontext.h"

namespace SE {

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
    
ShamanEngine::ShamanEngine()
{
    init();
}

ShamanEngine::~ShamanEngine()
{
    if (Config::get().enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
    }
}

std::vector<const char *> ShamanEngine::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (Config::get().enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void ShamanEngine::init()
{
    ctx = std::make_shared<VulkanContext>();
    ctx->Se_engine = this;
    ctx->Se_window = new SeWindow(ctx);
    createInstance();
    ctx->Se_window->createWindowSurface();
    setupDebugMessenger();
    ctx->Se_device = new SeDevice(ctx);
    ctx->Se_swapchain = new SeSwapChain(ctx);
    ctx->Se_renderer = new SeRenderer(ctx);
    ctx->Se_camera = new SeCamera(ctx);
  
}

void ShamanEngine::createInstance()
{
    if (Config::get().enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Shaman Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (Config::get().enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        std::cout << "failed to create instance!: " << instance << std::endl;
        throw std::runtime_error("failed to create instance!");
    }

    hasGflwRequiredInstanceExtensions();
}

void ShamanEngine::hasGflwRequiredInstanceExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    if (Config::get().print_extensions_to_console())
    {
        std::cout << "available extensions:" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto &extension : extensions) {
            std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }
    
        std::cout << "required extensions:" << std::endl;
        auto requiredExtensions = getRequiredExtensions();
        for (const auto &required : requiredExtensions) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing required glfw extension");
            }
        }
    }
}

bool ShamanEngine::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void ShamanEngine::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
}

void ShamanEngine::setupDebugMessenger() {
    if (!Config::get().enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS) {
        std::cout << "failed to set up debug messenger!" << std::endl;
        throw std::runtime_error("failed to set up debug messenger!");
    }
  
}


void ShamanEngine::run()
{
    auto cameraObject = SeObject::createObject();
    SeController* cameraController = new SeController(ctx);
    
    ctx->Se_camera->setViewDirection(glm::vec3(0.f), glm::vec3(1.5f, 0.f, 0.f));
    ctx->Se_camera->setViewTarget(glm::vec3(0.f, 0.f, -0.0000000001f), glm::vec3(0.0f, 0.0f, 0.f));
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    while (!ctx->Se_window->shouldClose())
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
        frameTime = glm::min(frameTime, Config::get().max_frame_time());
        currentTime = newTime;

        cameraController->moveInPlaneXZ(ctx->Se_window->window, frameTime, cameraObject);
        ctx->Se_camera->setViewYXZ(cameraObject.transform.translation, cameraObject.transform.rotation);
        
        float aspect = ctx->Se_swapchain->extentAspectRatio();
        ctx->Se_camera->setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);
        if (auto commandBuffer = ctx->Se_renderer->beginFrame())
        {
            ctx->Se_renderer->beginSwapChainRenderPass(commandBuffer);
            ctx->Se_renderer->renderObjects(commandBuffer, *ctx->Se_camera);
            ctx->Se_renderer->endSwapChainRenderPass(commandBuffer);
            ctx->Se_renderer->endFrame();
        }
    }
    vkDeviceWaitIdle(ctx->Se_device->device);
}


}
