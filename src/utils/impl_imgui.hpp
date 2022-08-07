#pragma once

#include <daxa/utils/imgui.hpp>
#include <deque>

namespace daxa
{
    struct ImplImGuiRenderer final : ManagedSharedState
    {
        ImGuiRendererInfo info;
        RasterPipeline raster_pipeline;
        BufferId vbuffer, staging_vbuffer;
        BufferId ibuffer, staging_ibuffer;

        std::unordered_map<u32, size_t> tex_handle_ptr_to_referenced_image_index;
        std::vector<ImageId> referenced_images;

        void recreate_vbuffer(usize vbuffer_new_size);
        void recreate_ibuffer(usize ibuffer_new_size);
        auto get_imgui_texture_id(ImageId img) -> u64;
        void record_commands(ImDrawData * draw_data, CommandList & cmd_list, ImageId target_image, u32 size_x, u32 size_y);
        auto managed_cleanup() -> bool override;

        ImplImGuiRenderer(ImGuiRendererInfo const & info);
        virtual ~ImplImGuiRenderer() override final;
    };
} // namespace daxa
