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

        void blit_image_to_image(ImageBlitInfo & info);
        void copy_image_to_image(ImageCopyInfo & info);
        void clear_image(ImageClearInfo const & info);

        void pipeline_barrier(PipelineBarrierInfo const & info);
        void pipeline_barrier_image_transition(PipelineBarrierImageTransitionInfo const & info);

        void complete();

      private:
        friend struct Device;
        CommandList(std::shared_ptr<void> impl);
    };
} // namespace daxa
