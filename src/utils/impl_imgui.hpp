#pragma once

#include <daxa/utils/imgui.hpp>
#include <deque>

namespace daxa
{
    struct ImplImGuiRenderer final : ManagedSharedState
    {
        ImGuiRendererInfo info;
        RasterPipeline raster_pipeline;
        BufferId vbuffer{};
        BufferId ibuffer{};
        ImageId font_sheet{};
        usize frame_count = {};

        void recreate_vbuffer(usize vbuffer_new_size);
        void recreate_ibuffer(usize ibuffer_new_size);
        void record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y);

        ImplImGuiRenderer(ImGuiRendererInfo a_info);
        ~ImplImGuiRenderer();
    };
} // namespace daxa
