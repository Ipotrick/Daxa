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
        u32 width = 0;
        u32 height = 0;
        std::function<i32(Format)> surface_format_selector = default_format_score;
        PresentMode present_mode = PresentMode::DOUBLE_BUFFER_WAIT_FOR_VBLANK;
        PresentOp present_operation = PresentOp::IDENTITY;
        ImageUsageFlags image_usage = {};
        usize max_allowed_frames_in_flight = 1;
        std::string debug_name = {};
    };

    struct Swapchain : ManagedPtr
    {
        auto info() const -> SwapchainInfo const &;
        auto get_format() const -> Format;
        void resize();
        auto acquire_next_image() -> ImageId;
        /// @brief The gpu needs to wait until the swapchain image is available. 
        //// The first submit that uses the acquired image must wait on this.
        /// @return the binary semaphore that needs to be waited on in the first use of the image.
        auto get_acquire_semaphore() -> BinarySemaphore &;
        /// @brief The gpu needs to wait until the swapchain image is presentable. 
        /// The LAST submit that uses the swapchain image must signal this semaphore.
        /// The present must wait on this semaphore.
        /// @return the binary semaphore that needs to be signaled on in the last use, waited on present.
        auto get_present_semaphore() -> BinarySemaphore &;
        /// @brief The swapchain needs to know when the last frame has ended because of some insane vulkan spec problems.
        /// On the last submit of a frame, signal this timeline semaphore to to the cpu frame value.
        /// @return the gpu timeline value that needs to be signaled in the end of the frame.
        auto get_gpu_timeline_semaphore() -> TimelineSemaphore &;
        // The current cpu framecount.
        auto get_cpu_timeline_value() -> usize;
        void change_present_mode(PresentMode new_present_mode);

      private:
        friend struct Device;
        explicit Swapchain(ManagedPtr impl);
    };
} // namespace daxa
