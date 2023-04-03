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
        std::function<i32(Format)> surface_format_selector = default_format_score;
        PresentMode present_mode = PresentMode::FIFO;
        PresentOp present_operation = PresentOp::IDENTITY;
        ImageUsageFlags image_usage = {};
        usize max_allowed_frames_in_flight = 2;
        std::string name = {};
    };

    struct Swapchain : ManagedPtr
    {
        Swapchain() = default;

        auto info() const -> SwapchainInfo const &;
        auto get_surface_extent() const -> Extent2D;
        auto get_format() const -> Format;
        void resize();
        /// @brief The ImageId may change between calls. This must be called to obtain a new swapchain image to be used for rendering.
        /// @return A swapchain image, that will be ready to render to when the acquire semaphore is signaled. This may return an empty image id if the swapchain is out of date.
        auto acquire_next_image() -> ImageId;
        /// The acquire semaphore must be waited on in the first submission that uses the last acquired image.
        /// This semaphore may change between acquires, so it needs to be re-queried after every get_acquire_semaphore call.
        /// @return The binary semaphore that is signaled when the last acquired image is ready to be used.
        auto get_acquire_semaphore() const -> BinarySemaphore const &;
        /// @brief The present semaphore must be signaled in the last submission that uses the last acquired swapchain image.
        /// The present semaphore must be waited on in the present of the last acquired image.
        /// This semaphore may change between acquires, so it needs to be re-queried after every get_acquire_semaphore call.
        /// @return The present semaphore that needs to be signaled and waited on for present of the last acquired image.
        auto get_present_semaphore() const -> BinarySemaphore const &;
        /// @brief The swapchain needs to know when the last use of the swapchain happens to limit the frames in flight.
        /// In the last submission that uses the swapchain image, signal this timeline semaphore with the cpu timeline value.
        /// @return the gpu timeline semaphore that needs to be signaled.
        auto get_gpu_timeline_semaphore() const -> TimelineSemaphore const &;
        /// @brief The last submission that uses the swapchain image needs to signal the timeline with the cpu value.
        /// @return The cpu frame timeline value.
        auto get_cpu_timeline_value() const -> usize;
        void change_present_mode(PresentMode new_present_mode);

      private:
        friend struct Device;
        explicit Swapchain(ManagedPtr impl);
    };
} // namespace daxa
