#include "impl_core.hpp"
#include "impl_swapchain.hpp"
#include "impl_device.hpp"

#include <utility>
#include <bit>

namespace daxa
{
    Swapchain::Swapchain(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}

    void Swapchain::resize()
    {
        auto & impl = *as<ImplSwapchain>();
        impl.recreate();
    }

    auto Swapchain::info() const -> SwapchainInfo const &
    {
        auto const & impl = *as<ImplSwapchain>();
        return *reinterpret_cast<SwapchainInfo const*>(&impl.info);
    }

    auto Swapchain::get_surface_extent() const -> Extent2D
    {
        auto const & impl = *as<ImplSwapchain>();
        return impl.surface_extent;
    }

    auto Swapchain::get_format() const -> Format
    {
        auto const & impl = *as<ImplSwapchain>();
        return static_cast<Format>(impl.vk_surface_format.format);
    }

    auto Swapchain::acquire_next_image() -> ImageId
    {
        auto & impl = *as<ImplSwapchain>();
        // A new frame starts.
        // We wait until the gpu timeline is frames of flight behind our cpu timeline value.
        // This will limit the frames in flight.
        impl.gpu_frame_timeline.wait_for_value(
            static_cast<u64>(
                std::max<i64>(
                    0,
                    static_cast<i64>(impl.cpu_frame_timeline) - static_cast<i64>(impl.info.max_allowed_frames_in_flight))));
        impl.acquire_semaphore_index = (impl.cpu_frame_timeline + 1) % impl.info.max_allowed_frames_in_flight;
        BinarySemaphore & acquire_semaphore = impl.acquire_semaphores[impl.acquire_semaphore_index];
        VkResult const err = vkAcquireNextImageKHR(
            impl.device->vk_device,
            impl.vk_swapchain, UINT64_MAX,
            acquire_semaphore.as<daxa_ImplBinarySemaphore>()->vk_semaphore,
            nullptr,
            &impl.current_image_index);
        DAXA_DBG_ASSERT_TRUE_M(
            err == VK_SUCCESS ||
                err == VK_ERROR_OUT_OF_DATE_KHR ||
                err == VK_ERROR_SURFACE_LOST_KHR ||
                err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
            "Daxa should never be in a situation where Acquire fails");

        if (err == VK_ERROR_OUT_OF_DATE_KHR ||
            err == VK_ERROR_SURFACE_LOST_KHR ||
            err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
        {
            // The swapchain needs recreation, we can only return a null ImageId here.
            return ImageId{};
        }
        // We only bump the cpu timeline, when the acquire succeeds.
        impl.cpu_frame_timeline += 1;
        return impl.images[impl.current_image_index];
    }

    auto Swapchain::get_acquire_semaphore() const -> BinarySemaphore const &
    {
        auto const & impl = *as<ImplSwapchain>();
        return impl.acquire_semaphores[impl.acquire_semaphore_index];
    }

    auto Swapchain::get_present_semaphore() const -> BinarySemaphore const &
    {
        auto const & impl = *as<ImplSwapchain>();
        return impl.present_semaphores[impl.current_image_index];
    }

    auto Swapchain::get_gpu_timeline_semaphore() const -> TimelineSemaphore const &
    {
        auto const & impl = *as<ImplSwapchain>();
        return impl.gpu_frame_timeline;
    }

    auto Swapchain::get_cpu_timeline_value() const -> usize
    {
        auto const & impl = *as<ImplSwapchain>();
        return impl.cpu_frame_timeline;
    }

    void ImplSwapchain::recreate_surface()
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

    auto ImplSwapchain::get_index_of_image(ImageId image) const -> usize
    {
        for (usize i = 0; i < images.size(); ++i)
        {
            if (images[i] == image)
            {
                return i;
            }
        }
        DAXA_DBG_ASSERT_TRUE_M(false, "tried to get swapchain index of an image not owned by this swapchain!");
        return std::numeric_limits<usize>::max();
    }

    ImplSwapchain::ImplSwapchain(daxa_Device a_device, daxa_SwapchainInfo a_info)
        : device{a_device},
          info{a_info}, // TODO(capi) needs to make std::string storage for name!!
          gpu_frame_timeline{TimelineSemaphore{ManagedPtr(
              daxa_ImplTimelineSemaphore::create(this->device, daxa_TimelineSemaphoreInfo{.initial_value = 0, .name = {/* this->info.name + " gpu timeline" */}}),
              [](daxa_ImplHandle * self_ptr)
              {
                  delete reinterpret_cast<daxa_ImplTimelineSemaphore *>(self_ptr);
              })}}
    {
        recreate_surface();

        u32 format_count = 0;

        auto const & device_impl = *this->device;

        vkGetPhysicalDeviceSurfaceFormatsKHR(device_impl.vk_physical_device, this->vk_surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats;
        surface_formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device_impl.vk_physical_device, this->vk_surface, &format_count, surface_formats.data());
        DAXA_DBG_ASSERT_TRUE_M(format_count > 0, "No formats found");

        auto format_comparator = [&](auto const & a, auto const & b) -> bool
        {
            return this->info.surface_format_selector(a.format) <
                   this->info.surface_format_selector(b.format);
        };
        auto best_format = std::max_element(surface_formats.begin(), surface_formats.end(), format_comparator);
        DAXA_DBG_ASSERT_TRUE_M(best_format != surface_formats.end(), "No viable formats found");
        this->vk_surface_format = *best_format;

        recreate();

        // We have an acquire semaphore for each frame in flight.
        for (u32 i = 0; i < this->info.max_allowed_frames_in_flight; i++)
        {
            acquire_semaphores.push_back(BinarySemaphore{
                ManagedPtr(
                    daxa_ImplBinarySemaphore::create(this->device, daxa_BinarySemaphoreInfo{.name = {/* this->info.name + ", image " + std::to_string(i) + " acquire semaphore" */}}),
                    [](daxa_ImplHandle * self_ptr)
                    {
                        delete reinterpret_cast<daxa_ImplBinarySemaphore *>(self_ptr);
                    }),
            });
        }
        // We have a present semaphore for each swapchain image.
        for (u32 i = 0; i < this->images.size(); i++)
        {
            present_semaphores.push_back(BinarySemaphore{
                ManagedPtr(
                    daxa_ImplBinarySemaphore::create(this->device, daxa_BinarySemaphoreInfo{.name = {/* this->info.name + ", image " + std::to_string(i) + " present semaphore" */}}),
                    [](daxa_ImplHandle * self_ptr)
                    {
                        delete reinterpret_cast<daxa_ImplBinarySemaphore *>(self_ptr);
                    }),
            });
        }
    }

    ImplSwapchain::~ImplSwapchain()
    {
        cleanup();

        vkDestroySwapchainKHR(this->device->vk_device, this->vk_swapchain, nullptr);
        vkDestroySurfaceKHR(this->device->instance->vk_instance, this->vk_surface, nullptr);
    }

    void ImplSwapchain::recreate()
    {
        // compare this->info with new info
        // we need to pass in the old swapchain handle to create a new one

        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->device->vk_physical_device, this->vk_surface, &surface_capabilities);
        if (surface_extent.x == surface_capabilities.currentExtent.width &&
            surface_extent.y == surface_capabilities.currentExtent.height &&
            this->vk_swapchain != VK_NULL_HANDLE)
        {
            return;
        }

        surface_extent.x = surface_capabilities.currentExtent.width;
        surface_extent.y = surface_capabilities.currentExtent.height;

#if __linux__
        // TODO: I (grundlett) am too lazy to find out why the other present modes
        // fail on Linux. This can be inspected by Linux people and they can
        // submit a PR if they find a fix.
        info.present_mode = PresentMode::IMMEDIATE;
#endif

        auto * old_swapchain = this->vk_swapchain;

        // NOTE: this is a hack that allows us to ignore issues caused
        // by things that are just underspecified in the Vulkan spec.
        daxa_dvc_wait_idle(this->device);

        cleanup();

        ImageUsageFlags const usage = std::bit_cast<ImageUsageFlags>(info.image_usage) | ImageUsageFlagBits::COLOR_ATTACHMENT;

        VkSwapchainCreateInfoKHR const swapchain_create_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = this->vk_surface,
            .minImageCount = 3,
            .imageFormat = this->vk_surface_format.format,
            .imageColorSpace = this->vk_surface_format.colorSpace,
            .imageExtent = VkExtent2D{surface_extent.x, surface_extent.y},
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

        vkCreateSwapchainKHR(
            this->device->vk_device,
            &swapchain_create_info,
            nullptr,
            &this->vk_swapchain);

        if (old_swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(this->device->vk_device, old_swapchain, nullptr);
        }

        u32 image_count = 0;
        std::vector<VkImage> swapchain_images;
        vkGetSwapchainImagesKHR(this->device->vk_device, vk_swapchain, &image_count, nullptr);
        swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(this->device->vk_device, vk_swapchain, &image_count, swapchain_images.data());
        this->images.resize(image_count);
        for (u32 i = 0; i < images.size(); i++)
        {
            ImageInfo const image_info = {
                .format = static_cast<Format>(this->vk_surface_format.format),
                .size = {this->surface_extent.x, this->surface_extent.y, 1},
                .usage = usage,
                // TODO(capi) needs a std::string backing
                .name = {}, /* this->info.name + " Image #" + std::to_string(i) */
            };
            this->images[i] = this->device->new_swapchain_image(
                swapchain_images[i], vk_surface_format.format, i, usage, image_info);
        }

        if ((this->device->instance->info.flags & DAXA_INSTANCE_FLAG_DEBUG_UTIL) != 0 && this->info.name.size != 0)
        {
            auto swapchain_name = std::string(this->info.name.data, this->info.name.size);
            VkDebugUtilsObjectNameInfoEXT const swapchain_name_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
                .objectHandle = reinterpret_cast<uint64_t>(this->vk_swapchain),
                .pObjectName = swapchain_name.c_str(),
            };
            this->device->vkSetDebugUtilsObjectNameEXT(this->device->vk_device, &swapchain_name_info);
        }
    }

    void ImplSwapchain::cleanup()
    {
        for (auto & image : images)
        {
            this->device->zombify_image(std::bit_cast<daxa_ImageId>(image));
        }
        images.clear();
    }
} // namespace daxa
