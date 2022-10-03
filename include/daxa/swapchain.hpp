#pragma once

#include <daxa/core.hpp>

#include <daxa/gpu_resources.hpp>
#include <daxa/semaphore.hpp>

namespace daxa
{
    static inline auto default_format_score(Format format) -> i32
    {
        switch (format)
        {
        case Format::R8G8B8A8_SRGB: return 90;
        case Format::R8G8B8A8_UNORM: return 80;
        case Format::B8G8R8A8_SRGB: return 70;
        case Format::B8G8R8A8_UNORM: return 60;
        default: return 0;
        }
    }

    struct SwapchainInfo
    {
        NativeWindowHandle native_window;
        NativeWindowPlatform native_window_platform;
        u32 width = 0;
        u32 height = 0;
        std::function<i32(Format)> surface_format_selector = default_format_score;
        PresentMode present_mode = PresentMode::DOUBLE_BUFFER_WAIT_FOR_VBLANK;
        PresentOp present_operation = PresentOp::IDENTITY;
        ImageUsageFlags image_usage = {};
        usize max_allowed_frames_in_flight = 2;
        std::string debug_name = {};
    };

    struct Swapchain : ManagedPtr
    {
        auto info() const -> SwapchainInfo const &;
        auto get_format() const -> Format;
        void resize();
        /// @brief The ImageId may change between calls. This must be called to optain a new swapchain image to be used for rendering.
        /// @return A swapchain image, that will be ready to render to when the acquire smaphore is signaled. This may return an empty image id if the swapchain is out of date.
        auto acquire_next_image() -> ImageId;
        /// @brief The gpu needs to wait until the swapchain image is available. 
        //// The first submit that uses the acquired image must wait on this.
        /// This semaphore may change between acquire calls.
        /// @return the binary semaphore that needs to be waited on in the first use of the currently acquired swapchain image.
        auto get_acquire_semaphore() -> BinarySemaphore &;
        /// @brief The gpu needs to wait until the swapchain image is presentable. 
        /// The LAST submit that uses the swapchain image must signal this semaphore.
        /// The present must wait on this semaphore.
        /// This semaphore may change between acquire calls.
        /// @return the binary semaphore that needs to be signaled on in the last use, waited on present.
        auto get_present_semaphore() -> BinarySemaphore &;
        /// @brief The swapchain needs to know when the last frame has ended because of some insane vulkan spec problems.
        /// In the last submission that uses the swapchain image, signal this timeline semaphore with the cpu timeline value.
        /// @return the gpu timeline semaphore that needs to be signaled.
        auto get_gpu_timeline_semaphore() -> TimelineSemaphore &;
        /// @brief The last submission that uses the swapchain image needs to signal the timeline with the cpu value.
        /// @return The cpu frame timeline value.
        auto get_cpu_timeline_value() -> usize;
        void change_present_mode(PresentMode new_present_mode);

      private:
        friend struct Device;
        explicit Swapchain(ManagedPtr impl);
    };
} // namespace daxa
