#pragma once

#include "../impl_core.hpp"

#include <daxa/utils/imgui.hpp>
#include <deque>

namespace daxa
{
    struct ImplImGuiRenderer final : ImplHandle
    {
        ImGuiRendererInfo info = {};
        RasterPipeline raster_pipeline = {};
        BufferId vbuffer = {};
        BufferId ibuffer = {};
        ImageId font_sheet = {};
        SamplerId font_sampler = {};
        usize frame_count = {};

        std::vector<ImGuiImageContext> image_sampler_pairs = {};

        void recreate_vbuffer(usize vbuffer_new_size);
        void recreate_ibuffer(usize ibuffer_new_size);
        void record_commands(ImDrawData * draw_data, CommandRecorder & recorder, ImageId target_image, u32 size_x, u32 size_y);

        ImplImGuiRenderer(ImGuiRendererInfo a_info);
        ~ImplImGuiRenderer();

        static void zero_ref_callback(ImplHandle const *);
    };
} // namespace daxa
