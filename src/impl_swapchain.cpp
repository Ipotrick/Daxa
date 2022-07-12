// #define

#include "impl_swapchain.hpp"

namespace daxa
{
    Swapchain::Swapchain(std::shared_ptr<void> impl) : Handle(impl) {}

    ImplSwapchain::ImplSwapchain(std::shared_ptr<ImplDevice> impl_device, SwapchainInfo const & info)
        : impl_device{impl_device}
    {
#if defined(_WIN32)
        VkWin32SurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .hinstance = GetModuleHandleA(nullptr),
            .hwnd = info.native_window_handle,
        };
        vkCreateWin32SurfaceKHR(impl_device->ctx->vk_instance_handle, &surface_ci, nullptr, &this->vk_surface_handle);
#elif defined(__linux__)
        VkXlibSurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .dpy = nullptr,
            .window = info.native_window_handle,
        };
        vkCreateXlibSurfaceKHR(impl_device->ctx->vk_instance_handle, &surface_ci, nullptr, &this->vk_swapchain_handle);
#endif

        u32 format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(impl_device->vk_physical_device, this->vk_surface_handle, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats;
        surface_formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(impl_device->vk_physical_device, this->vk_surface_handle, &format_count, surface_formats.data());

        auto format_score = [&](VkSurfaceFormatKHR surface_format) -> i32
        {
            switch (static_cast<daxa::Format>(surface_format.format))
            {
            case Format::A2B10G10R10_UNORM_PACK32: return 100;
            case Format::R8G8B8A8_SRGB: return 90;
            case Format::R8G8B8A8_UNORM: return 80;
            case Format::B8G8R8A8_SRGB: return 70;
            case Format::B8G8R8A8_UNORM: return 60;
            default: return 0;
            }
        };

        auto format_comparator = [&](auto const & a, auto const & b) -> bool
        {
            return format_score(a) < format_score(b);
        };
        auto best_format = std::max_element(surface_formats.begin(), surface_formats.end(), format_comparator);
        this->vk_surface_format = *best_format;

        recreate(info);
    }

    ImplSwapchain::~ImplSwapchain()
    {
    }

    void ImplSwapchain::recreate(SwapchainInfo const & info)
    {
        // compare this->info with new info
        // we need to pass in the old swapchain handle to create a new one

        auto old_swapchain_handle = this->vk_swapchain_handle;

        cleanup();

        VkSwapchainCreateInfoKHR swapchain_create_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = this->vk_surface_handle,
            .minImageCount = 3,
            .imageFormat = this->vk_surface_format.format,
            .imageColorSpace = this->vk_surface_format.colorSpace,
            .imageExtent = VkExtent2D{info.width, info.height},
            .imageArrayLayers = 1,
            .imageUsage = info.image_usage | ImageUsageFlagBits::COLOR_ATTACHMENT,
            .imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &this->impl_device->main_queue_family_index,
            .preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(info.present_operation),
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = static_cast<VkPresentModeKHR>(info.present_mode),
            .clipped = VK_TRUE,
            .oldSwapchain = old_swapchain_handle,
        };

        impl_device->volk_device_table.vkCreateSwapchainKHR(
            impl_device->vk_device_handle,
            &swapchain_create_info,
            nullptr,
            &this->vk_swapchain_handle);

        if (old_swapchain_handle != VK_NULL_HANDLE)
        {
            impl_device->volk_device_table.vkDestroySwapchainKHR(impl_device->vk_device_handle, old_swapchain_handle, nullptr);
        }

        u32 image_count;
        std::vector<VkImage> swapchain_images;
        impl_device->volk_device_table.vkGetSwapchainImagesKHR(impl_device->vk_device_handle, vk_swapchain_handle, &image_count, nullptr);
        swapchain_images.resize(image_count);
        impl_device->volk_device_table.vkGetSwapchainImagesKHR(impl_device->vk_device_handle, vk_swapchain_handle, &image_count, swapchain_images.data());
        this->image_resources.resize(image_count);
        for (u32 i = 0; i < image_resources.size(); i++) {
            this->image_resources[i].vk_image = swapchain_images[i];
            // impl_device->volk_device_table.vkCreateImageView(impl_device->vk_device_handle, &color_image_view, nullptr, &image_resources[i].view);
        }
    }

    void ImplSwapchain::cleanup()
    {
        for (size_t i = 0; i < image_resources.size(); i++)
        {
            // vkDestroyImageView(device_handle, image_resources[i].view, nullptr);
            // image_resources[i].view = VK_NULL_HANDLE;
        }
    }
} // namespace daxa