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
        SamplerId default_sampler = {};
        usize frame_count = {};

        void recreate_vbuffer(usize vbuffer_new_size);
        void recreate_ibuffer(usize ibuffer_new_size);
        void update_textures(ImDrawData * draw_data, CommandRecorder & recorder);
        void record_commands(ImGuiRecordCommandsInfo const & record_info);
        void delete_texture(ImTextureData * tex);

        ImplImGuiRenderer(ImGuiRendererInfo a_info);
        ~ImplImGuiRenderer();

        static void zero_ref_callback(ImplHandle const *);
    };
} // namespace daxa
