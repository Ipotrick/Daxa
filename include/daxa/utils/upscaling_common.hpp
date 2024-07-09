#pragma once

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    struct UpscaleSizeInfo
    {
        u32 render_size_x, render_size_y;
        u32 display_size_x, display_size_y;
    };

    struct UpscaleInstanceInfo
    {
        Device device;
        UpscaleSizeInfo size_info = {};
        bool depth_inf = false;
        bool depth_inv = false;
        bool color_hdr = false;
    };

    struct UpscaleInfo
    {
        ImageId color, depth, motion_vectors;
        // ImageId reactive, trans_and_comp;
        ImageId output;

        bool should_reset = false;
        f32 delta_time;
        daxa_f32vec2 jitter;

        bool should_sharpen = false;
        f32 sharpening = 0.0f;

        struct CameraInfo
        {
            f32 near_plane = {};
            f32 far_plane = {};
            f32 vertical_fov = {};
        };
        CameraInfo camera_info = {};
    };
} // namespace daxa
