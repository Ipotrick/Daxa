#include "impl_core.hpp"
#include "impl_swapchain.hpp"
#include "impl_device.hpp"

#include <utility>
#include <bit>

// --- Begin API Functions ---

auto daxa_default_format_selector(VkFormat format) -> i32
{
    switch (format)
    {
    case VK_FORMAT_R8G8B8A8_SRGB: return 90;
    case VK_FORMAT_R8G8B8A8_UNORM: return 80;
    case VK_FORMAT_B8G8R8A8_SRGB: return 70;
    case VK_FORMAT_B8G8R8A8_UNORM: return 60;
    default: return 0;
    }
}

auto daxa_dvc_create_swapchain(daxa_Device device, daxa_SwapchainInfo const * info, daxa_Swapchain * out_swapchain) -> daxa_Result
{
    auto ret = daxa_ImplSwapchain{};
    ret.device = device;
    ret.info = *reinterpret_cast<SwapchainInfo const *>(info);
    ret.info_name = {ret.info.name.data(), ret.info.name.size()};
    ret.info.name = {ret.info_name.data(), ret.info_name.size()};
    ret.recreate_surface();

    u32 format_count = 0;

    auto vk_result = vkGetPhysicalDeviceSurfaceFormatsKHR(ret.device->vk_physical_device, ret.vk_surface, &format_count, nullptr);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    std::vector<VkSurfaceFormatKHR> surface_formats;
    surface_formats.resize(format_count);
    vk_result = vkGetPhysicalDeviceSurfaceFormatsKHR(ret.device->vk_physical_device, ret.vk_surface, &format_count, surface_formats.data());
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    if (format_count == 0)
    {
        return DAXA_RESULT_NO_SUITABLE_FORMAT_FOUND;
    }

    auto format_comparator = [&](auto const & a, auto const & b) -> bool
    {
        return ret.info.surface_format_selector(std::bit_cast<Format>(a.format)) <
               ret.info.surface_format_selector(std::bit_cast<Format>(b.format));
    };
    auto best_format = std::max_element(surface_formats.begin(), surface_formats.end(), format_comparator);
    if (best_format == surface_formats.end())
    {
        return DAXA_RESULT_NO_SUITABLE_FORMAT_FOUND;
    }
    ret.vk_surface_format = *best_format;
    ret.recreate();
    // We have an acquire semaphore for each frame in flight.
    for (u32 i = 0; i < ret.info.max_allowed_frames_in_flight; i++)
    {
        BinarySemaphore sema = {};
        daxa_BinarySemaphoreInfo sema_info = {};
        auto result = daxa_dvc_create_binary_semaphore(device, &sema_info, reinterpret_cast<daxa_BinarySemaphore *>(&sema));
        if (result != DAXA_RESULT_SUCCESS)
        {
            return result;
        }
        ret.acquire_semaphores.push_back(std::move(sema));
    }
    // We have a present semaphore for each swapchain image.
    for (u32 i = 0; i < ret.images.size(); i++)
    {
        BinarySemaphore sema = {};
        daxa_BinarySemaphoreInfo sema_info = {};
        auto result = daxa_dvc_create_binary_semaphore(device, &sema_info, reinterpret_cast<daxa_BinarySemaphore *>(&sema));
        if (result != DAXA_RESULT_SUCCESS)
        {
            return result;
        }
        ret.present_semaphores.push_back(std::move(sema));
    }
    *out_swapchain = new daxa_ImplSwapchain{};
    **out_swapchain = std::move(ret);
    device->inc_weak_refcnt();
    return DAXA_RESULT_SUCCESS;
}

auto daxa_swp_get_surface_extent(daxa_Swapchain self) -> VkExtent2D
{
    return self->surface_extent;
}

auto daxa_swp_get_format(daxa_Swapchain self) -> VkFormat
{
    return self->vk_surface_format.format;
}

auto daxa_swp_resize(daxa_Swapchain self) -> daxa_Result
{
    return self->recreate();
}

auto daxa_swp_acquire_next_image(daxa_Swapchain self, daxa_ImageId * out_image_id) -> daxa_Result
{
    self->gpu_frame_timeline.wait_for_value(
        static_cast<u64>(
            std::max<i64>(
                0,
                static_cast<i64>(self->cpu_frame_timeline) - static_cast<i64>(self->info.max_allowed_frames_in_flight))));
    self->acquire_semaphore_index = (self->cpu_frame_timeline + 1) % self->info.max_allowed_frames_in_flight;
    BinarySemaphore & acquire_semaphore = self->acquire_semaphores[self->acquire_semaphore_index];
    auto result = vkAcquireNextImageKHR(
        self->device->vk_device,
        self->vk_swapchain, UINT64_MAX,
        acquire_semaphore.as<daxa_ImplBinarySemaphore>()->vk_semaphore,
        nullptr,
        &self->current_image_index);

    // TODO(capi): port to cpp wrapper:
    // if (err == VK_ERROR_OUT_OF_DATE_KHR ||
    //     err == VK_ERROR_SURFACE_LOST_KHR ||
    //     err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
    // {
    // }
    // We only bump the cpu timeline, when the acquire succeeds.
    self->cpu_frame_timeline += 1;
    *out_image_id = std::bit_cast<daxa_ImageId>(self->images[self->current_image_index]);
    return std::bit_cast<daxa_Result>(result);
}

auto daxa_swp_get_acquire_semaphore(daxa_Swapchain self) -> daxa_BinarySemaphore
{
    return *self->acquire_semaphores[self->acquire_semaphore_index].as<daxa_BinarySemaphore>();
}

auto daxa_swp_get_present_semaphore(daxa_Swapchain self) -> daxa_BinarySemaphore
{
    return *self->present_semaphores[self->current_image_index].as<daxa_BinarySemaphore>();
}

auto daxa_swp_get_gpu_timeline_semaphore(daxa_Swapchain self) -> daxa_TimelineSemaphore
{
    return *self->gpu_frame_timeline.as<daxa_TimelineSemaphore>();
}

auto daxa_swp_get_cpu_timeline_value(daxa_Swapchain self) -> usize
{
    return self->cpu_frame_timeline;
}

auto daxa_swp_info(daxa_Swapchain self) -> daxa_SwapchainInfo const *
{
    return reinterpret_cast<daxa_SwapchainInfo const *>(&self->info);
}

auto daxa_swp_get_vk_swapchain(daxa_Swapchain self) -> VkSwapchainKHR
{
    return self->vk_swapchain;
}

auto daxa_swp_get_vk_surface(daxa_Swapchain self) -> VkSurfaceKHR
{
    return self->vk_surface;
}

auto daxa_swp_inc_refcnt(daxa_Swapchain self) -> u64
{
    return self->inc_refcnt();
}

auto daxa_swp_dec_refcnt(daxa_Swapchain self) -> u64
{
    return self->dec_refcnt(
        &daxa_ImplSwapchain::zero_ref_callback,
        self->device->instance
    );
}

// --- End API Functions ---

// --- Begin Internals ---

auto daxa_ImplSwapchain::recreate() -> daxa_Result
{
    // compare this->info with new info
    // we need to pass in the old swapchain handle to create a new one

    VkSurfaceCapabilitiesKHR surface_capabilities;
    auto vk_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        this->device->vk_physical_device,
        this->vk_surface,
        &surface_capabilities);
    DAXA_DBG_ASSERT_TRUE_M(vk_result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
    if (surface_extent.width == surface_capabilities.currentExtent.width &&
        surface_extent.height == surface_capabilities.currentExtent.height &&
        this->vk_swapchain != VK_NULL_HANDLE)
    {
        return DAXA_RESULT_SUCCESS;
    }

    surface_extent.width = surface_capabilities.currentExtent.width;
    surface_extent.height = surface_capabilities.currentExtent.height;

#if __linux__
    // TODO(grundlett): I (grundlett) am too lazy to find out why the other present modes
    // fail on Linux. This can be inspected by Linux people and they can
    // submit a PR if they find a fix.
    info.present_mode = PresentMode::IMMEDIATE;
#endif

    auto * old_swapchain = this->vk_swapchain;

    // NOTE: this is a hack that allows us to ignore issues caused
    // by things that are just underspecified in the Vulkan spec.
    auto result = daxa_dvc_wait_idle(this->device);
    if (result != DAXA_RESULT_SUCCESS)
    {
        return result;
    }

    this->cleanup();

    ImageUsageFlags const usage = std::bit_cast<ImageUsageFlags>(info.image_usage) | ImageUsageFlagBits::COLOR_ATTACHMENT;

    VkSwapchainCreateInfoKHR const swapchain_create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = this->vk_surface,
        .minImageCount = 3,
        .imageFormat = this->vk_surface_format.format,
        .imageColorSpace = this->vk_surface_format.colorSpace,
        .imageExtent = surface_extent,
        .imageArrayLayers = 1,
        .imageUsage = usage.data,
        .imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &this->device->main_queue_family_index,
        .preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(info.present_operation),
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = static_cast<VkPresentModeKHR>(info.present_mode),
        .clipped = VK_TRUE,
        .oldSwapchain = old_swapchain,
    };

    vk_result = vkCreateSwapchainKHR(
        this->device->vk_device,
        &swapchain_create_info,
        nullptr,
        &this->vk_swapchain);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }

    if (old_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(this->device->vk_device, old_swapchain, nullptr);
    }

    u32 image_count = 0;
    std::vector<VkImage> swapchain_images;
    vk_result = vkGetSwapchainImagesKHR(this->device->vk_device, vk_swapchain, &image_count, nullptr);
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    swapchain_images.resize(image_count);
    vk_result = vkGetSwapchainImagesKHR(this->device->vk_device, vk_swapchain, &image_count, swapchain_images.data());
    if (vk_result != VK_SUCCESS)
    {
        return std::bit_cast<daxa_Result>(vk_result);
    }
    this->images.resize(image_count);
    for (u32 i = 0; i < images.size(); i++)
    {
        ImageInfo const image_info = {
            .format = static_cast<Format>(this->vk_surface_format.format),
            .size = {this->surface_extent.width, this->surface_extent.height, 1},
            .usage = usage,
            .name = this->info_name.c_str(),
        };
        auto result_pair = this->device->new_swapchain_image(
            swapchain_images[i], vk_surface_format.format, i, usage, image_info);
        if (result_pair.first != DAXA_RESULT_SUCCESS)
        {
            return result_pair.first;
        }
        this->images[i] = result_pair.second;
    }

    if ((this->device->instance->info.flags & InstanceFlagBits::DEBUG_UTILS) != InstanceFlagBits::NONE && this->info_name.size() != 0)
    {
        VkDebugUtilsObjectNameInfoEXT const swapchain_name_info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
            .objectHandle = reinterpret_cast<u64>(this->vk_swapchain),
            .pObjectName = this->info_name.c_str(),
        };
        this->device->vkSetDebugUtilsObjectNameEXT(this->device->vk_device, &swapchain_name_info);
    }
    return DAXA_RESULT_SUCCESS;
}

void daxa_ImplSwapchain::cleanup()
{
    for (auto & image : images)
    {
        daxa_dvc_dec_refcnt_image(this->device, std::bit_cast<daxa_ImageId>(image));
    }
    images.clear();
}

void daxa_ImplSwapchain::recreate_surface()
{
    if (this->vk_surface != nullptr)
    {
        vkDestroySurfaceKHR(this->device->instance->vk_instance, this->vk_surface, nullptr);
    }
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR const surface_ci{
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = static_cast<HWND>(info.native_window),
    };
    {
        auto func = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(this->device->instance->vk_instance, "vkCreateWin32SurfaceKHR"));
        func(this->device->instance->vk_instance, &surface_ci, nullptr, &this->vk_surface);
    }
#elif defined(__linux__)
    switch (this->info.native_window_platform)
    {
#if DAXA_BUILT_WITH_WAYLAND
    case NativeWindowPlatform::WAYLAND_API:
    {
        // TODO(grundlett): figure out how to link Wayland
        VkWaylandSurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .display = wl_display_connect(nullptr),
            .surface = static_cast<wl_surface *>(this->info.native_window),
        };
        {
            auto func = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(vkGetInstanceProcAddr(impl_device.as<ImplDevice>()->impl_ctx.as<ImplInstance>()->vk_instance, "vkCreateWaylandSurfaceKHR"));
            func(impl_device.as<ImplDevice>()->impl_ctx.as<ImplInstance>()->vk_instance, &surface_ci, nullptr, &this->vk_surface);
        }
    }
    break;
#endif
#if DAXA_BUILT_WITH_X11
    case NativeWindowPlatform::XLIB_API:
    default:
    {
        VkXlibSurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .dpy = XOpenDisplay(nullptr),
            .window = reinterpret_cast<Window>(this->info.native_window),
        };
        {
            auto func = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(vkGetInstanceProcAddr(impl_device.as<ImplDevice>()->impl_ctx.as<ImplInstance>()->vk_instance, "vkCreateXlibSurfaceKHR"));
            func(impl_device.as<ImplDevice>()->impl_ctx.as<ImplInstance>()->vk_instance, &surface_ci, nullptr, &this->vk_surface);
        }
    }
    break;
    }
#endif
#endif
}

void daxa_ImplSwapchain::zero_ref_callback(daxa_ImplHandle * handle)
{
    // TODO: Dont we need to defer the destruction with a zombie?
    auto self = r_cast<daxa_Swapchain>(handle);
    self->cleanup();
    vkDestroySwapchainKHR(self->device->vk_device, self->vk_swapchain, nullptr);
    vkDestroySurfaceKHR(self->device->instance->vk_instance, self->vk_surface, nullptr);
    self->device->dec_weak_refcnt(
        daxa_ImplDevice::zero_ref_callback,
        self->device->instance
    );
    delete self;
}

// --- End Internals ---