#pragma once

#include <daxa/core.hpp>

#include <daxa/gpu_resources.hpp>

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
        std::string debug_name = {};
    };

    struct Swapchain : Handle
    {
        auto info() const -> SwapchainInfo const &;

        ImageId acquire_next_image();
        void resize(u32 width, u32 height);
        void change_present_mode(PresentMode new_present_mode);

      private:
        friend struct Device;
        Swapchain(std::shared_ptr<void> impl);
    };
} // namespace daxa
