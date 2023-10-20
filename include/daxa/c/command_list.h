#ifndef __DAXA_COMMAND_LIST_H__
#define __DAXA_COMMAND_LIST_H__

#include <daxa/c/types.h>
#include <daxa/c/sync.h>
#include <daxa/c/gpu_resources.h>
#include <daxa/c/pipeline.h>

typedef struct
{
    daxa_StringView name;
} daxa_CommandListInfo;

typedef struct
{
    daxa_ImageId src_image;
    daxa_ImageLayout src_image_layout;
    daxa_ImageId dst_image;
    daxa_ImageLayout dst_image_layout;
    daxa_ImageArraySlice src_slice;
    VkOffset3D src_offsets[2];
    daxa_ImageArraySlice dst_slice;
    VkOffset3D dst_offsets[2];
    VkFilter filter;
} daxa_ImageBlitInfo;

static const daxa_ImageBlitInfo DAXA_DEFAULT_IMAGE_BLIT_INFO = {
    .src_image = {},
    .src_image_layout = DAXA_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    .dst_image = {},
    .dst_image_layout = DAXA_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .src_slice = {},
    .src_offsets = {},
    .dst_slice = {},
    .dst_offsets = {},
    .filter = VK_FILTER_NEAREST,
};

typedef struct
{
    daxa_BufferId src_buffer;
    daxa_BufferId dst_buffer;
    size_t src_offset;
    size_t dst_offset;
    size_t size;
} daxa_BufferCopyInfo;

static const daxa_BufferCopyInfo DAXA_DEFAULT_BUFFER_COPY_INFO = {};

typedef struct
{
    daxa_BufferId buffer;
    size_t buffer_offset;
    daxa_ImageId image;
    daxa_ImageLayout image_layout;
    daxa_ImageArraySlice image_slice;
    VkOffset3D image_offset;
    VkExtent3D image_extent;
} daxa_BufferImageCopyInfo;

static const daxa_BufferImageCopyInfo DAXA_DEFAULT_BUFFER_IMAGE_COPY_INFO = {
    .buffer = {},
    .buffer_offset = 0,
    .image = {},
    .image_layout = DAXA_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .image_slice = {},
    .image_offset = {},
    .image_extent = {},
};

typedef struct
{
    daxa_ImageId image;
    daxa_ImageLayout image_layout;
    daxa_ImageArraySlice image_slice;
    VkOffset3D image_offset;
    VkExtent3D image_extent;
    daxa_BufferId buffer;
    size_t buffer_offset;
} daxa_ImageBufferCopyInfo;

static const daxa_ImageBufferCopyInfo DAXA_DEFAULT_IMAGE_BUFFER_COPY_INFO = {
    .image = {},
    .image_layout = DAXA_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    .image_slice = {},
    .image_offset = {},
    .image_extent = {},
    .buffer = {},
    .buffer_offset = 0,
};

typedef struct
{
    daxa_ImageId src_image;
    daxa_ImageLayout src_image_layout;
    daxa_ImageId dst_image;
    daxa_ImageLayout dst_image_layout;
    daxa_ImageArraySlice src_slice;
    VkOffset3D src_offset;
    daxa_ImageArraySlice dst_slice;
    VkOffset3D dst_offset;
    VkExtent3D extent;
} daxa_ImageCopyInfo;

static const daxa_ImageCopyInfo DAXA_DEFAULT_IMAGE_COPY_INFO = {
    .src_image = {},
    .src_image_layout = DAXA_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    .dst_image = {},
    .dst_image_layout = DAXA_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .src_slice = {},
    .src_offset = {},
    .dst_slice = {},
    .dst_offset = {},
    .extent = {},
};

// Make sure this stays abi compatible with daxa::ClearValue
_DAXA_DECL_VARIANT(VkClearValue)

typedef struct
{
    daxa_ImageLayout image_layout;
    daxa_Variant(VkClearValue) clear_value;
    daxa_ImageId image;
    daxa_ImageMipArraySlice dst_slice;
} daxa_ImageClearInfo;

static const daxa_ImageClearInfo DAXA_DEFAULT_IMAGE_CLEAR_INFO = {
    .image_layout = DAXA_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .clear_value = {},
    .image = {},
    .dst_slice = {},
};

typedef struct
{
    daxa_BufferId buffer;
    size_t offset;
    size_t size;
    uint32_t clear_value;
} daxa_BufferClearInfo;

static const daxa_BufferClearInfo DAXA_DEFAULT_BUFFER_CLEAR_INFO = {};

typedef struct
{
    daxa_ImageViewId image_view;
    daxa_ImageLayout layout;
    VkAttachmentLoadOp load_op;
    VkAttachmentStoreOp store_op;
    daxa_Variant(VkClearValue) clear_value;
} daxa_RenderAttachmentInfo;
_DAXA_DECL_OPTIONAL(daxa_RenderAttachmentInfo)
_DAXA_DECL_FIXED_LIST(daxa_RenderAttachmentInfo, 8)

static const daxa_RenderAttachmentInfo DAXA_DEFAULT_RENDER_ATTACHMENT_INFO = {
    .image_view = {},
    .layout = DAXA_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    .load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .store_op = VK_ATTACHMENT_STORE_OP_STORE,
    .clear_value = {},
};

typedef struct
{
    daxa_FixedList(daxa_RenderAttachmentInfo, 8) color_attachments;
    daxa_Optional(daxa_RenderAttachmentInfo) depth_attachment;
    daxa_Optional(daxa_RenderAttachmentInfo) stencil_attachment;
    VkRect2D render_area;
} daxa_RenderPassBeginInfo;

static const daxa_RenderPassBeginInfo DAXA_DEFAULT_RENDERPASS_BEGIN_INFO = {};

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t offset;
} daxa_DispatchIndirectInfo;

static const daxa_DispatchIndirectInfo DAXA_DEFAULT_DISPATCH_INDIRECT_INFO = {};

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t offset;
    uint32_t draw_count;
    uint32_t stride;
} daxa_DrawMeshTasksIndirectInfo;

static const daxa_DrawMeshTasksIndirectInfo DAXA_DEFAULT_DRAW_MESH_TASKS_INDIRECT_INFO = {
    .indirect_buffer = {},
    .offset = 0,
    .draw_count = 1,
    .stride = 12,
};

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t offset;
    daxa_BufferId count_buffer;
    size_t count_offset;
    uint32_t max_count;
    uint32_t stride;
} daxa_DrawMeshTasksIndirectCountInfo;

static const daxa_DrawMeshTasksIndirectCountInfo DAXA_DRAW_MESH_TASKS_INDIRECT_COUNT_INFO = {
    .indirect_buffer = {},
    .offset = 0,
    .count_buffer = {},
    .count_offset = 0,
    .max_count = 0,
    .stride = 12,
};

typedef struct
{
    uint32_t vertex_count;
    uint32_t instance_count;
    uint32_t first_vertex;
    uint32_t first_instance;
} daxa_DrawInfo;

static const daxa_DrawInfo DAXA_DEFAULT_DRAW_INFO = {
    .vertex_count = 0,
    .instance_count = 1,
    .first_vertex = 0,
    .first_instance = 0,
};

typedef struct
{
    uint32_t index_count;
    uint32_t instance_count;
    uint32_t first_index;
    int32_t vertex_offset;
    uint32_t first_instance;
} daxa_DrawIndexedInfo;

static const daxa_DrawIndexedInfo DAXA_DEFAULT_DRAW_INDEXED_INFO = {
    .index_count = 0,
    .instance_count = 1,
    .first_index = 0,
    .vertex_offset = 0,
    .first_instance = 0,
};

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t indirect_buffer_offset;
    uint32_t draw_count;
    uint32_t draw_command_stride;
    daxa_Bool8 is_indexed;
} daxa_DrawIndirectInfo;

static const daxa_DrawIndirectInfo DAXA_DEFAULT_DRAW_INDIRECT_INFO = {
    .indirect_buffer = {},
    .indirect_buffer_offset = 0,
    .draw_count = 1,
    .draw_command_stride = 0,
    .is_indexed = 0,
};

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t indirect_buffer_offset;
    daxa_BufferId count_buffer;
    size_t count_buffer_offset;
    uint32_t max_draw_count;
    uint32_t draw_command_stride;
    daxa_Bool8 is_indexed;
} daxa_DrawIndirectCountInfo;

static const daxa_DrawIndirectCountInfo DAXA_DEFAULT_DRAW_INDIRECT_COUNT_INFO = {
    .indirect_buffer = {},
    .indirect_buffer_offset = 0,
    .count_buffer = {},
    .count_buffer_offset = 0,
    .max_draw_count = ((1 << 16) - 1),
    .draw_command_stride = 0,
    .is_indexed = 0,
};

typedef struct
{
    daxa_Event * event;
    VkPipelineStageFlags stage;
} daxa_ResetEventsInfo;

typedef struct
{
    daxa_Event * events;
    size_t event_count;
} daxa_WaitEventsInfo;

typedef struct
{
    daxa_TimelineQueryPool * query_pool;
    VkPipelineStageFlags2 pipeline_stage;
    uint32_t query_index;
} daxa_WriteTimestampInfo;

typedef struct
{
    daxa_TimelineQueryPool * query_pool;
    uint32_t start_index;
    uint32_t count;
} daxa_ResetTimestampsInfo;

typedef struct
{
    daxa_f32vec4 label_color;
    daxa_StringView name;
} daxa_CommandLabelInfo;

typedef struct
{
    daxa_Event * barrier;
    VkPipelineStageFlags stage_masks;
} daxa_ResetEventInfo;

typedef struct
{
    // Binding slot the buffer will be bound to.
    uint32_t slot;
    daxa_BufferId buffer;
    size_t size;
    size_t offset;
} daxa_SetUniformBufferInfo;

static const daxa_SetUniformBufferInfo DAXA_DEFAULT_SET_UNIFORM_BUFFER_INFO = {};

typedef struct
{
    float constant_factor;
    float clamp;
    float slope_factor;
} daxa_DepthBiasInfo;

static const daxa_DepthBiasInfo DAXA_DEFAULT_DEPTH_BIAS_INFO = {};

typedef struct
{
    daxa_BufferId buffer;
    size_t offset;
    VkIndexType index_type;
} daxa_SetIndexBufferInfo;

static const daxa_SetIndexBufferInfo DAXA_DEFAULT_SET_INDEX_BUFFER_INFO = {
    .buffer = {},
    .offset = 0,
    .index_type = VK_INDEX_TYPE_UINT32,
};

DAXA_EXPORT daxa_Result
daxa_cmd_copy_buffer_to_buffer(daxa_CommandList cmd_list, daxa_BufferCopyInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_copy_buffer_to_image(daxa_CommandList cmd_list, daxa_BufferImageCopyInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_copy_image_to_buffer(daxa_CommandList cmd_list, daxa_ImageBufferCopyInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_copy_image_to_image(daxa_CommandList cmd_list, daxa_ImageCopyInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_blit_image_to_image(daxa_CommandList cmd_list, daxa_ImageBlitInfo const * info);

DAXA_EXPORT daxa_Result
daxa_cmd_clear_buffer(daxa_CommandList cmd_list, daxa_BufferClearInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_clear_image(daxa_CommandList cmd_list, daxa_ImageClearInfo const * info);

/// @brief  Successive pipeline barrier calls are combined.
///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
/// @param info parameters.
DAXA_EXPORT void
daxa_cmd_pipeline_barrier(daxa_CommandList cmd_list, daxa_MemoryBarrierInfo const * info);
/// @brief  Successive pipeline barrier calls are combined.
///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
/// @param info parameters.
DAXA_EXPORT daxa_Result
daxa_cmd_pipeline_barrier_image_transition(daxa_CommandList cmd_list, daxa_ImageMemoryBarrierInfo const * info);
DAXA_EXPORT void
daxa_cmd_signal_event(daxa_CommandList cmd_list, daxa_EventSignalInfo const * info);
DAXA_EXPORT void
daxa_cmd_wait_events(daxa_CommandList cmd_list, daxa_EventWaitInfo const * infos, size_t info_count);
DAXA_EXPORT void
daxa_cmd_wait_event(daxa_CommandList cmd_list, daxa_EventWaitInfo const * info);
DAXA_EXPORT void
daxa_cmd_reset_event(daxa_CommandList cmd_list, daxa_ResetEventInfo const * info);

DAXA_EXPORT void
daxa_cmd_push_constant(daxa_CommandList cmd_list, void const * data, uint32_t size);
/// @brief  Binds a buffer region to the uniform buffer slot.
///         There are 8 uniform buffer slots (indices range from 0 to 7).
///         The buffer range is user managed, The buffer MUST not be destroyed before the command list is submitted!
///         Changes to these bindings only become visible to commands AFTER a pipeline is bound!
///         This is in stark contrast to OpenGl like bindings which are visible immediately to all commands after binding.
///         This is deliberate to discourage overuse of uniform buffers over descriptor sets.
///         Set uniform buffer slots are cleared after a pipeline is bound.
///         Before setting another pipeline, they need to be set again.
/// @param info parameters.
DAXA_EXPORT daxa_Result
daxa_cmd_set_uniform_buffer(daxa_CommandList cmd_list, daxa_SetUniformBufferInfo const * info);
DAXA_EXPORT void
daxa_cmd_set_compute_pipeline(daxa_CommandList cmd_list, daxa_ComputePipeline pipeline);
DAXA_EXPORT void
daxa_cmd_set_raster_pipeline(daxa_CommandList cmd_list, daxa_RasterPipeline pipeline);
DAXA_EXPORT void
daxa_cmd_dispatch(daxa_CommandList cmd_list, uint32_t x, uint32_t y, uint32_t z);
DAXA_EXPORT daxa_Result
daxa_cmd_dispatch_indirect(daxa_CommandList cmd_list, daxa_DispatchIndirectInfo const * info);

/// @brief  Destroys the buffer AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id buffer to be destroyed after command list finishes.
DAXA_EXPORT daxa_Result
daxa_cmd_destroy_buffer_deferred(daxa_CommandList cmd_list, daxa_BufferId id);
/// @brief  Destroys the image AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image to be destroyed after command list finishes.
DAXA_EXPORT daxa_Result
daxa_cmd_destroy_image_deferred(daxa_CommandList cmd_list, daxa_ImageId id);
/// @brief  Destroys the image view AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image view to be destroyed after command list finishes.
DAXA_EXPORT daxa_Result
daxa_cmd_destroy_image_view_deferred(daxa_CommandList cmd_list, daxa_ImageViewId id);
/// @brief  Destroys the sampler AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image sampler be destroyed after command list finishes.
DAXA_EXPORT daxa_Result
daxa_cmd_destroy_sampler_deferred(daxa_CommandList cmd_list, daxa_SamplerId id);

/// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
///         Between the begin and end renderpass commands, the renderpass persists and draw-calls can be recorded.
/// @param info parameters.
DAXA_EXPORT daxa_Result
daxa_cmd_begin_renderpass(daxa_CommandList cmd_list, daxa_RenderPassBeginInfo const * info);
/// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
///         Between the begin and end renderpass commands, the renderpass persists and draw-calls can be recorded.
DAXA_EXPORT void
daxa_cmd_end_renderpass(daxa_CommandList cmd_list);
DAXA_EXPORT void
daxa_cmd_set_viewport(daxa_CommandList cmd_list, VkViewport const * info);
DAXA_EXPORT void
daxa_cmd_set_scissor(daxa_CommandList cmd_list, VkRect2D const * info);
DAXA_EXPORT void
daxa_cmd_set_depth_bias(daxa_CommandList cmd_list, daxa_DepthBiasInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_set_index_buffer(daxa_CommandList cmd_list, daxa_SetIndexBufferInfo const * info);

DAXA_EXPORT void
daxa_cmd_draw(daxa_CommandList cmd_list, daxa_DrawInfo const * info);
DAXA_EXPORT void
daxa_cmd_draw_indexed(daxa_CommandList cmd_list, daxa_DrawIndexedInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_draw_indirect(daxa_CommandList cmd_list, daxa_DrawIndirectInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_draw_indirect_count(daxa_CommandList cmd_list, daxa_DrawIndirectCountInfo const * info);
DAXA_EXPORT void
daxa_cmd_draw_mesh_tasks(daxa_CommandList cmd_list, uint32_t x, uint32_t y, uint32_t z);
DAXA_EXPORT daxa_Result
daxa_cmd_draw_mesh_tasks_indirect(daxa_CommandList cmd_list, daxa_DrawMeshTasksIndirectInfo const * info);
DAXA_EXPORT daxa_Result
daxa_cmd_draw_mesh_tasks_indirect_count(daxa_CommandList cmd_list, daxa_DrawMeshTasksIndirectCountInfo const * info);

DAXA_EXPORT void
daxa_cmd_write_timestamp(daxa_CommandList cmd_list, daxa_WriteTimestampInfo const * info);
DAXA_EXPORT void
daxa_cmd_reset_timestamps(daxa_CommandList cmd_list, daxa_ResetTimestampsInfo const * info);

DAXA_EXPORT void
daxa_cmd_begin_label(daxa_CommandList cmd_list, daxa_CommandLabelInfo const * info);
DAXA_EXPORT void
daxa_cmd_end_label(daxa_CommandList cmd_list);

// Is called by all other commands. Flushes internal pipeline barrier list to actual vulkan call.
DAXA_EXPORT void
daxa_cmd_flush_barriers(daxa_CommandList cmd_list);
DAXA_EXPORT daxa_Result
daxa_cmd_complete(daxa_CommandList cmd_list);
DAXA_EXPORT daxa_Bool8
daxa_cmd_is_complete(daxa_CommandList cmd_list);
DAXA_EXPORT daxa_CommandListInfo const *
daxa_cmd_info(daxa_CommandList cmd_list);
DAXA_EXPORT VkCommandBuffer
daxa_cmd_get_vk_command_buffer(daxa_CommandList cmd_list);
DAXA_EXPORT VkCommandPool
daxa_cmd_get_vk_command_pool(daxa_CommandList cmd_list);
DAXA_EXPORT uint64_t
daxa_cmd_inc_refcnt(daxa_CommandList command_list);
DAXA_EXPORT uint64_t
daxa_cmd_dec_refcnt(daxa_CommandList command_list);

#endif // #ifndef __DAXA_COMMAND_LIST_H__
