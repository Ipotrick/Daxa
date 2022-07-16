#pragma once

#include <memory>

#include <daxa/gpu_resources.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/swapchain.hpp>

namespace daxa
{
    struct CommandListInfo
    {
        std::string debug_name = {};
    };

    struct ImageBlitInfo
    {
        ImageId src_image = {};
        ImageLayout src_image_layout = {};
        ImageId dst_image = {};
        ImageLayout dst_image_layout = {};
        ImageArraySlice src_slice = {};
        std::array<Offset3D, 2> src_offsets = {};
        ImageArraySlice dst_slice = {};
        std::array<Offset3D, 2> dst_offsets = {};
        Filter filter = {};
    };

    struct ImageCopyInfo
    {
        ImageId src_image = {};
        ImageLayout src_image_layout = {};
        ImageId dst_image = {};
        ImageLayout dst_image_layout = {};
        ImageArraySlice src_slice = {};
        Offset3D src_offset = {};
        ImageArraySlice dst_slice = {};
        Offset3D dst_offset = {};
        Extent3D extent = {};
    };

    struct ImageClearInfo
    {
        ImageLayout dst_image_layout = {};
        ClearColor clear_color;
        ImageId dst_image = {};
        ImageMipArraySlice dst_slice = {};
    };

    struct PipelineBarrierInfo
    {
        PipelineStageAccessFlags awaited_pipeline_access = PipelineStageAccessFlagBits::NONE;
        PipelineStageAccessFlags waiting_pipeline_access = PipelineStageAccessFlagBits::NONE;
    };

    struct PipelineBarrierImageTransitionInfo
    {
        PipelineStageAccessFlags awaited_pipeline_access = PipelineStageAccessFlagBits::NONE;
        PipelineStageAccessFlags waiting_pipeline_access = PipelineStageAccessFlagBits::NONE;
        ImageLayout before_layout = ImageLayout::UNDEFINED;
        ImageLayout after_layout = ImageLayout::UNDEFINED;
        ImageId image_id = {};
        ImageMipArraySlice image_slice = {};
    };

    struct CommandList : Handle
    {
        ~CommandList();

        void blit_image_to_image(ImageBlitInfo const & info);
        void copy_image_to_image(ImageCopyInfo const & info);
        void clear_image(ImageClearInfo const & info);

        void pipeline_barrier(PipelineBarrierInfo const & info);
        void pipeline_barrier_image_transition(PipelineBarrierImageTransitionInfo const & info);

        void push_constant(void const * data, u32 size, u32 offset = 0);
        template <typename T>
        void push_constant(T const & constant, usize offset = 0)
        {
            push_constant(&constant, static_cast<u32>(sizeof(T)), static_cast<u32>(offset));
        }
        void bind_pipeline(ComputePipeline const & compute_pipeline);
        void dispatch(u32 group_x, u32 group_y = 1, u32 group_z = 1);

        void complete();

      private:
        friend struct Device;
        CommandList(std::shared_ptr<void> impl);
    };
} // namespace daxa
