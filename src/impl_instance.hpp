#pragma once

#include "impl_core.hpp"
#include "impl_features.hpp"

#include <daxa/c/instance.h>

#ifdef STREAMLINE_ENABLED
#include <daxa/utils/streamline.hpp>
#endif // STREAMLINE_ENABLED

struct ImplPhysicalDevice
{
    PhysicalDeviceExtensionsStruct extensions = {};
    PhysicalDeviceFeaturesStruct features = {};
    VkPhysicalDevice vk_handle = {};
};

struct daxa_ImplInstance final : ImplHandle
{
    InstanceInfo info = {};
    std::string engine_name = {};
    std::string app_name = {};
    VkInstance vk_instance = {};
#ifdef STREAMLINE_ENABLED
    // Streamline
    bool sl_enabled = false;
    std::vector<sl::Feature> sl_active_features = {};
    daxa::StreamlineContext streamline;
#endif // STREAMLINE_ENABLED

    std::vector<ImplPhysicalDevice> device_internals = {};
    std::vector<daxa_DeviceProperties> device_properties = {};

#if DAXA_USE_DYNAMIC_VULKAN
    // dynamic lib pointer
    HMODULE vulkan_lib;

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;

    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;

    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
    PFN_vkCreateDevice vkCreateDevice;
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
    PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
    PFN_vkCreateShaderModule vkCreateShaderModule;
    PFN_vkDestroyShaderModule vkDestroyShaderModule;
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;

    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
#endif 

    static void zero_ref_callback(ImplHandle const * handle);

    auto initialize_physical_devices() -> daxa_Result;

    auto load_global_functions(const char* preferred_lib = nullptr) -> bool;
    auto load_instance_functions() -> bool;
};