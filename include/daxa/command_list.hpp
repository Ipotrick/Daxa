#pragma once

#include <daxa/core.hpp>
#include <daxa/gpu_resources.hpp>
#include <daxa/pipeline.hpp>

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

    struct BufferCopyInfo
    {
        BufferId src_buffer = {};
        usize src_offset = {};
        BufferId dst_buffer = {};
        usize dst_offset = {};
        usize size = {};
    }; 

    struct BufferImageCopy
    {
        BufferId buffer = {};
        usize buffer_offset = {};
        ImageId image = {};
        ImageLayout image_layout = {};
        ImageArraySlice image_slice = {};
        Offset3D image_offset = {};
        Extent3D image_extent = {};
    };

    struct ImageToBufferInfo
    {
        ImageId image = {};
        ImageLayout image_layout = {};
        ImageArraySlice image_slice = {};
        Offset3D image_offset = {};
        Extent3D image_extent = {};
        BufferId buffer = {};
        usize buffer_offset = {};
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

        void copy_buffer_to_buffer(BufferCopyInfo const & info);
        void copy_buffer_to_image(BufferImageCopy const & info);
        void copy_image_to_buffer(BufferImageCopy const & info);
        void copy_image_to_image(ImageCopyInfo const & info);
        void blit_image_to_image(ImageBlitInfo const & info);
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

        void destroy_buffer_deferred(BufferId id);
        void destroy_image_deferred(ImageId id);
        void destroy_image_view_deferred(ImageViewId id);
        void destroy_sampler_deferred(SamplerId id);

        void complete();

      private:
        friend struct Device;
        CommandList(std::shared_ptr<void> impl);
    };
} // namespace daxa
