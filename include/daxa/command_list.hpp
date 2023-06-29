#pragma once

#include <daxa/core.hpp>
#include <daxa/gpu_resources.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/split_barrier.hpp>

#include <span>
#include <daxa/timeline_query.hpp>

namespace daxa
{
    static inline constexpr usize CONSTANT_BUFFER_BINDINGS_COUNT = 8;

    struct CommandListInfo
    {
        std::string name = {};
    };

    struct ImageBlitInfo
    {
        ImageId src_image = {};
        ImageLayout src_image_layout = ImageLayout::TRANSFER_SRC_OPTIMAL;
        ImageId dst_image = {};
        ImageLayout dst_image_layout = ImageLayout::TRANSFER_DST_OPTIMAL;
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

    struct BufferImageCopyInfo
    {
        BufferId buffer = {};
        usize buffer_offset = {};
        ImageId image = {};
        ImageLayout image_layout = ImageLayout::TRANSFER_DST_OPTIMAL;
        ImageArraySlice image_slice = {};
        Offset3D image_offset = {};
        Extent3D image_extent = {};
    };

    struct ImageBufferCopyInfo
    {
        ImageId image = {};
        ImageLayout image_layout = ImageLayout::TRANSFER_SRC_OPTIMAL;
        ImageArraySlice image_slice = {};
        Offset3D image_offset = {};
        Extent3D image_extent = {};
        BufferId buffer = {};
        usize buffer_offset = {};
    };

    struct ImageCopyInfo
    {
        ImageId src_image = {};
        ImageLayout src_image_layout = daxa::ImageLayout::TRANSFER_SRC_OPTIMAL;
        ImageId dst_image = {};
        ImageLayout dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL;
        ImageArraySlice src_slice = {};
        Offset3D src_offset = {};
        ImageArraySlice dst_slice = {};
        Offset3D dst_offset = {};
        Extent3D extent = {};
    };

    struct ImageClearInfo
    {
        ImageLayout dst_image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL;
        ClearValue clear_value;
        ImageId dst_image = {};
        ImageMipArraySlice dst_slice = {};
    };

    struct BufferClearInfo
    {
        BufferId buffer = {};
        usize offset = {};
        usize size = {};
        u32 clear_value = {};
    };

    struct RenderAttachmentInfo
    {
        ImageViewId image_view{};
        ImageLayout layout = ImageLayout::ATTACHMENT_OPTIMAL;
        AttachmentLoadOp load_op = AttachmentLoadOp::DONT_CARE;
        AttachmentStoreOp store_op = AttachmentStoreOp::STORE;
        ClearValue clear_value = {};
    };

    struct RenderPassBeginInfo
    {
        std::vector<RenderAttachmentInfo> color_attachments = {};
        std::optional<RenderAttachmentInfo> depth_attachment = {};
        std::optional<RenderAttachmentInfo> stencil_attachment = {};
        Rect2D render_area = {};
    };

    struct DispatchIndirectInfo
    {
        BufferId indirect_buffer = {};
        usize offset = {};
    };

    struct DrawInfo
    {
        u32 vertex_count = {};
        u32 instance_count = 1;
        u32 first_vertex = {};
        u32 first_instance = {};
    };

    struct DrawIndexedInfo
    {
        u32 index_count = {};
        u32 instance_count = 1;
        u32 first_index = {};
        i32 vertex_offset = {};
        u32 first_instance = {};
    };

    struct DrawIndirectInfo
    {
        BufferId draw_command_buffer = {};
        usize draw_command_buffer_read_offset = {};
        u32 draw_count = 1;
        u32 draw_command_stride = {};
        bool is_indexed = {};
    };

    struct DrawIndirectCountInfo
    {
        BufferId draw_command_buffer = {};
        usize draw_command_buffer_read_offset = {};
        BufferId draw_count_buffer = {};
        usize draw_count_buffer_read_offset = {};
        u32 max_draw_count = static_cast<u32>(std::numeric_limits<u16>::max());
        u32 draw_command_stride = {};
        bool is_indexed = {};
    };

    struct ResetSplitBarriersInfo
    {
        SplitBarrierState & split_barrier;
        PipelineStageFlags stage = {};
    };

    struct WaitSplitBarriersInfo
    {
        std::span<SplitBarrierState> split_barriers = {};
    };

    struct WriteTimestampInfo
    {
        TimelineQueryPool & query_pool;
        PipelineStageFlags pipeline_stage = {};
        u32 query_index = {};
    };

    struct ResetTimestampsInfo
    {
        TimelineQueryPool & query_pool;
        u32 start_index = {};
        u32 count = {};
    };

    struct CommandLabelInfo
    {
        std::string label_name = {};
        std::array<f32, 4> label_color = {0.463f, 0.333f, 0.671f, 1.0f};
    };

    struct ResetSplitBarrierInfo
    {
        SplitBarrierState & barrier;
        PipelineStageFlags stage_masks = {};
    };

    struct SetConstantBufferInfo
    {
        // Binding slot the buffer will be bound to.
        u32 slot = {};
        BufferId buffer = {};
        usize size = {};
        usize offset = {};
    };

    struct DepthBiasInfo
    {
        f32 constant_factor = {};
        f32 clamp = {};
        f32 slope_factor = {};
    };

    struct CommandList : ManagedPtr
    {
        CommandList() = default;

        void copy_buffer_to_buffer(BufferCopyInfo const & info);
        void copy_buffer_to_image(BufferImageCopyInfo const & info);
        void copy_image_to_buffer(ImageBufferCopyInfo const & info);
        void copy_image_to_image(ImageCopyInfo const & info);
        void blit_image_to_image(ImageBlitInfo const & info);

        void clear_buffer(BufferClearInfo const & info);
        void clear_image(ImageClearInfo const & info);

        void pipeline_barrier(MemoryBarrierInfo const & info);
        void pipeline_barrier_image_transition(ImageBarrierInfo const & info);
        void signal_split_barrier(SplitBarrierSignalInfo const & info);
        void wait_split_barriers(std::span<SplitBarrierWaitInfo const> const & infos);
        void wait_split_barrier(SplitBarrierWaitInfo const & info);
        void reset_split_barrier(ResetSplitBarrierInfo const & info);

        void push_constant_vptr(void const * data, u32 size, u32 offset = 0);
        template <typename T>
        void push_constant(T const & constant, usize offset = 0)
        {
            push_constant_vptr(&constant, static_cast<u32>(sizeof(T)), static_cast<u32>(offset));
        }
        /// @brief  Binds a buffer region to the uniform buffer slot.
        ///         There are uniform buffer slots 0-7.
        ///         The buffer range is user managed, The buffer MUST not die while in use on the gpu!
        ///         Changes to these bindings only become visible to commands AFTER a pipeline is bound!
        ///         Set uniform buffer slots are cleared after a pipeline is bound. 
        ///         Before setting another pipeline, they need to be set again.
        /// @param info parameters.
        void set_uniform_buffer(SetConstantBufferInfo const & info);
        void set_pipeline(ComputePipeline const & pipeline);
        void set_pipeline(RasterPipeline const & pipeline);
        void dispatch(u32 group_x, u32 group_y = 1, u32 group_z = 1);
        void dispatch_indirect(DispatchIndirectInfo const & info);

        void destroy_buffer_deferred(BufferId id);
        void destroy_image_deferred(ImageId id);
        void destroy_image_view_deferred(ImageViewId id);
        void destroy_sampler_deferred(SamplerId id);

        void begin_renderpass(RenderPassBeginInfo const & info);
        void end_renderpass();
        void set_viewport(ViewportInfo const & info);
        void set_scissor(Rect2D const & info);
        void set_depth_bias(DepthBiasInfo const & info);
        void set_index_buffer(BufferId id, usize offset, usize index_type_byte_size = sizeof(u32));

        void draw(DrawInfo const & info);
        void draw_indexed(DrawIndexedInfo const & info);
        void draw_indirect(DrawIndirectInfo const & info);
        void draw_indirect_count(DrawIndirectCountInfo const & info);

        void write_timestamp(WriteTimestampInfo const & info);
        void reset_timestamps(ResetTimestampsInfo const & info);

        void begin_label(CommandLabelInfo const & info);
        void end_label();

        void complete();
        auto is_complete() const -> bool;

        auto info() const -> CommandListInfo const &;

      private:
        friend struct Device;
        explicit CommandList(ManagedPtr impl);
    };
} // namespace daxa
