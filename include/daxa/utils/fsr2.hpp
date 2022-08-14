#pragma once

#if !DAXA_BUILT_WITH_UTILS
#error "[package management error] You must build Daxa with the UTILS option enabled"
#endif

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    struct Fsr2SizeInfo
    {
        u32 render_size_x, render_size_y;
        u32 display_size_x, display_size_y;
    };

    struct Fsr2ContextInfo
    {
        Device device;
        Fsr2SizeInfo size_info = {};
    };

    struct Fsr2UpscaleInfo
    {
        ImageId color, depth, motion_vectors;
        ImageId reactive, trans_and_comp;
        ImageId output;

        bool should_reset = false;
        f32 delta_time;
        f32vec2 jitter;

        bool should_sharpen = false;
        f32 sharpening = 0.0f;

        struct Camera
        {
            f32 near_plane;
            f32 far_plane;
            f32 vertical_fov;
        };
        Camera camera;
    };

    struct Fsr2Context : ManagedPtr
    {
        Fsr2Context(Fsr2ContextInfo const & info);
        ~Fsr2Context();

        void resize(Fsr2SizeInfo const & info);
        void upscale(CommandList & command_list, Fsr2UpscaleInfo const & info);
        auto get_jitter(u64 index) const -> f32vec2;
    };
} // namespace daxa
