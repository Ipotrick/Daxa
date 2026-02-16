#ifndef __DAXA_SWAPCHAIN_H__
#define __DAXA_SWAPCHAIN_H__

#include <daxa/c/gpu_resources.h>
#include <daxa/c/sync.h>
#include <vulkan/vulkan_core.h>

DAXA_EXPORT int32_t
daxa_default_format_selector(VkFormat format);

typedef struct 
{
    void * hwnd;
} daxa_NativeWindowInfoWin32;

typedef struct
{
    void * window;
} daxa_NativeWindowInfoXlib;

typedef struct
{
    void* display;
    void* surface;
    daxa_u32 width;
    daxa_u32 height;
} daxa_NativeWindowInfoWayland;

typedef union
{
    daxa_NativeWindowInfoWin32 win32;
    daxa_NativeWindowInfoXlib xlib;
    daxa_NativeWindowInfoWayland wayland;
} daxa_NativeWindowInfoUnion;

typedef enum
{
    DAXA_NATIVE_WINDOW_PLATFORM_INDEX_WIN32 = 0,
    DAXA_NATIVE_WINDOW_PLATFORM_INDEX_XLIB = 1,
    DAXA_NATIVE_WINDOW_PLATFORM_INDEX_WAYLAND = 2,
    DAXA_NATIVE_WINDOW_PLATFORM_INDEX_MAX_ENUM = 0x7fffffff,
} daxa_NativeWindowPlatformIndex;

typedef daxa_Variant(daxa_NativeWindowInfoUnion) daxa_NativeWindowInfo;

typedef struct
{
    daxa_NativeWindowInfo native_window_info;
    int32_t (*surface_format_selector)(VkFormat);
    VkPresentModeKHR present_mode;
    VkSurfaceTransformFlagBitsKHR present_operation;
    daxa_ImageUsageFlags image_usage;
    size_t max_allowed_frames_in_flight;
    daxa_QueueFamily queue_family;
    daxa_SmallString name;
} daxa_SwapchainInfo;

DAXA_EXPORT VkExtent2D
daxa_swp_get_surface_extent(daxa_Swapchain swapchain);
DAXA_EXPORT VkFormat
daxa_swp_get_format(daxa_Swapchain swapchain);
DAXA_EXPORT VkColorSpaceKHR
daxa_swp_get_color_space(daxa_Swapchain swapchain);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_swp_resize(daxa_Swapchain swapchain);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_swp_set_present_mode(daxa_Swapchain swapchain, VkPresentModeKHR present_mode);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_swp_wait_for_next_frame(daxa_Swapchain swapchain);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_swp_acquire_next_image(daxa_Swapchain swapchain, daxa_ImageId * out_image);
DAXA_EXPORT daxa_BinarySemaphore *
daxa_swp_current_acquire_semaphore(daxa_Swapchain swapchain);
DAXA_EXPORT daxa_BinarySemaphore *
daxa_swp_current_present_semaphore(daxa_Swapchain swapchain);
DAXA_EXPORT uint64_t
daxa_swp_current_cpu_timeline_value(daxa_Swapchain swapchain);
DAXA_EXPORT daxa_TimelineSemaphore *
daxa_swp_gpu_timeline_semaphore(daxa_Swapchain swapchain);

DAXA_EXPORT daxa_SwapchainInfo const *
daxa_swp_info(daxa_Swapchain swapchain);

DAXA_EXPORT VkSwapchainKHR
daxa_swp_get_vk_swapchain(daxa_Swapchain swapchain);
DAXA_EXPORT VkSurfaceKHR
daxa_swp_get_vk_surface(daxa_Swapchain swapchain);
DAXA_EXPORT uint64_t
daxa_swp_inc_refcnt(daxa_Swapchain swapchain);
DAXA_EXPORT uint64_t
daxa_swp_dec_refcnt(daxa_Swapchain swapchain);

#endif // #ifndef __DAXA_SWAPCHAIN_H__
