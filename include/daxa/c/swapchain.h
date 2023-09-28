#ifndef __DAXA_SWAPCHAIN_H__
#define __DAXA_SWAPCHAIN_H__

#include <daxa/c/gpu_resources.h>
// #include <daxa/c/semaphore.h>

int32_t daxa_default_format_selector(VkFormat format);

typedef struct
{
    // TODO...
    // daxa_NativeWindowHandle native_window;
    // daxa_NativeWindowPlatform native_window_platform;

    int32_t (*surface_format_selector)(VkFormat);
    VkPresentModeKHR present_mode;
    VkSurfaceTransformFlagBitsKHR present_operation;
    daxa_ImageUsageFlags image_usage;
    size_t max_allowed_frames_in_flight;
    char const * name;
} daxa_SwapchainInfo;

typedef struct daxa_ImplSwapchain * daxa_Swapchain;

VkExtent2D
daxa_swp_get_surface_extent(daxa_Swapchain swapchain);
VkFormat
daxa_swp_get_format(daxa_Swapchain swapchain);
void daxa_swp_resize(daxa_Swapchain swapchain);

// TODO: these 6 functions lack their arguments, and I didn't take time to consider if they should return result
daxa_Result
daxa_swp_acquire_next_image(daxa_Swapchain swapchain);
daxa_Result
daxa_swp_get_acquire_semaphore(daxa_Swapchain swapchain);
daxa_Result
daxa_swp_get_present_semaphore(daxa_Swapchain swapchain);
daxa_Result
daxa_swp_get_gpu_timeline_semaphore(daxa_Swapchain swapchain);
daxa_Result
daxa_swp_get_cpu_timeline_value(daxa_Swapchain swapchain);
daxa_Result
daxa_swp_change_present_mode(daxa_Swapchain swapchain);

daxa_SwapchainInfo const *
daxa_swp_info(daxa_Swapchain swapchain);

VkSwapchainKHR
daxa_swp_get_vk_swapchain(daxa_Swapchain swapchain);
VkSurfaceKHR
daxa_swp_get_vk_surface(daxa_Swapchain swapchain);

#endif // #ifndef __DAXA_SWAPCHAIN_H__
