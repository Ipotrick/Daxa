#pragma once

#include <daxa/core.hpp>
#include <daxa/gpu_resources.hpp>
#include <daxa/pipeline.hpp>
#include <daxa/sync.hpp>

#include <span>

namespace daxa
{
    static inline constexpr usize CONSTANT_BUFFER_BINDINGS_COUNT = 8;

    struct CommandListInfo
    {
        std::string_view name = "";
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
        BufferId dst_buffer = {};
        usize src_offset = {};
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
        ClearValue clear_value = {};
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
        ImageViewId image_view = {};
        ImageLayout layout = ImageLayout::ATTACHMENT_OPTIMAL;
        AttachmentLoadOp load_op = AttachmentLoadOp::DONT_CARE;
        AttachmentStoreOp store_op = AttachmentStoreOp::STORE;
        ClearValue clear_value = {}; // TODO: might be incompatible c abi with daxa_ClearValue!
    };

    struct RenderPassBeginInfo
    {
        FixedList<RenderAttachmentInfo, 8> color_attachments = {};
        Optional<RenderAttachmentInfo> depth_attachment = {};
        Optional<RenderAttachmentInfo> stencil_attachment = {};
        Rect2D render_area = {};
    };

    struct DispatchIndirectInfo
    {
        BufferId indirect_buffer = {};
        usize offset = {};
    };

    struct DrawMeshTasksIndirectInfo
    {
        BufferId indirect_buffer = {};
        usize offset = {};
        u32 draw_count = 1;
        u32 stride = 12;
    };

    struct DrawMeshTasksIndirectCountInfo
    {
        BufferId indirect_buffer = {};
        usize offset = {};
        BufferId count_buffer = {};
        usize count_offset = {};
        u32 max_count = {};
        u32 stride = 12;
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

    struct ResetEventInfo
    {
        Event & event;
        PipelineStageFlags stage = {};
    };

    struct WaitEventInfo
    {
        std::span<Event> events = {};
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
        std::array<f32, 4> label_color = {0.463f, 0.333f, 0.671f, 1.0f};
        std::string_view name = "";
    };

    struct SetUniformBufferInfo
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

    enum struct IndexType
    {
        uint64 = 0,
        uint32 = 1,
        uint8 = 1000265000,
        none = 1000165000,
    };

    struct SetIndexBufferInfo
    {
        BufferId id = {};
        usize offset = {};
        IndexType index_type = IndexType::uint32;
    };

    struct CommandList final : ManagedPtr<CommandList>
    {
        CommandList() = default;

        void copy_buffer_to_buffer(BufferCopyInfo const & info);
        void copy_buffer_to_image(BufferImageCopyInfo const & info);
        void copy_image_to_buffer(ImageBufferCopyInfo const & info);
        void copy_image_to_image(ImageCopyInfo const & info);
        void blit_image_to_image(ImageBlitInfo const & info);
        void clear_buffer(BufferClearInfo const & info);
        void clear_image(ImageClearInfo const & info);

        /// @brief  Successive pipeline barrier calls are combined.
        ///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
        /// @param info parameters.
        void pipeline_barrier(MemoryBarrierInfo const & info);
        /// @brief  Successive pipeline barrier calls are combined.
        ///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
        /// @param info parameters.
        void pipeline_barrier_image_transition(ImageMemoryBarrierInfo const & info);
        void signal_event(EventSignalInfo const & info);
        void wait_events(std::span<EventWaitInfo const> const & infos);
        void wait_event(EventWaitInfo const & info);
        void reset_event(ResetEventInfo const & info);

        void push_constant_vptr(void const * data, u32 size);
        template <typename T>
        void push_constant(T const & constant)
        {
            push_constant_vptr(&constant, static_cast<u32>(sizeof(T)));
        }
        /// @brief  Binds a buffer region to the uniform buffer slot.
        ///         There are 8 uniform buffer slots (indices range from 0 to 7).
        ///         The buffer range is user managed, The buffer MUST not be destroyed before the command list is submitted!
        ///         Changes to these bindings only become visible to commands AFTER a pipeline is bound!
        ///         This is in stark contrast to OpenGl like bindings wich are visible immediately to all commands after binding.
        ///         This is deliberate to discourage overuse of uniform buffers over descriptor sets.
        ///         Set uniform buffer slots are cleared after a pipeline is bound.
        ///         Before setting another pipeline, they need to be set again.
        /// @param info parameters.
        void set_uniform_buffer(SetUniformBufferInfo const & info);
        void set_pipeline(ComputePipeline const & pipeline);
        void set_pipeline(RasterPipeline const & pipeline);
        void dispatch(u32 x, u32 y = 1, u32 z = 1);
        void dispatch_indirect(DispatchIndirectInfo const & info);

        /// @brief  Destroyes the buffer AFTER the gpu is finished executing the command list.
        ///         Useful for large uploads exceeding staging memory pools.
        /// @param id buffer to be destroyed after command list finishes.
        void destroy_buffer_deferred(BufferId id);
        /// @brief  Destroyes the image AFTER the gpu is finished executing the command list.
        ///         Useful for large uploads exceeding staging memory pools.
        /// @param id image to be destroyed after command list finishes.
        void destroy_image_deferred(ImageId id);
        /// @brief  Destroyes the image view AFTER the gpu is finished executing the command list.
        ///         Useful for large uploads exceeding staging memory pools.
        /// @param id image view to be destroyed after command list finishes.
        void destroy_image_view_deferred(ImageViewId id);
        /// @brief  Destroyes the sampler AFTER the gpu is finished executing the command list.
        ///         Useful for large uploads exceeding staging memory pools.
        /// @param id image sampler be destroyed after command list finishes.
        void destroy_sampler_deferred(SamplerId id);

        /// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
        ///         Between the begin and end renderpass commands, the renderpass persists and drawcalls can be recorded.
        /// @param info parameters.
        void begin_renderpass(RenderPassBeginInfo const & info);
        /// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
        ///         Between the begin and end renderpass commands, the renderpass persists and drawcalls can be recorded.
        void end_renderpass();
        void set_viewport(ViewportInfo const & info);
        void set_scissor(Rect2D const & info);
        void set_depth_bias(DepthBiasInfo const & info);
        void set_index_buffer(SetIndexBufferInfo const & info);

        void draw(DrawInfo const & info);
        void draw_indexed(DrawIndexedInfo const & info);
        void draw_indirect(DrawIndirectInfo const & info);
        void draw_indirect_count(DrawIndirectCountInfo const & info);
        void draw_mesh_tasks(u32 x, u32 y, u32 z);
        void draw_mesh_tasks_indirect(DrawMeshTasksIndirectInfo const & info);
        void draw_mesh_tasks_indirect_count(DrawMeshTasksIndirectCountInfo const & info);

        void write_timestamp(WriteTimestampInfo const & info);
        void reset_timestamps(ResetTimestampsInfo const & info);

        void begin_label(CommandLabelInfo const & info);
        void end_label();

        void complete();
        auto is_complete() const -> bool;

        auto info() const -> CommandListInfo const &;

      protected:
        template <typename T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };
} // namespace daxa
