#ifndef __DAXA_COMMAND_LIST_H__
#define __DAXA_COMMAND_LIST_H__

#include "daxa/c/core.h"
#include "daxa/c/gpu_resources.h"
#include "daxa/c/pipeline.h"
#include "daxa/c/event.h"
#include "daxa/c/timeline_query.h"

#define CONSTANT_BUFFER_BINDINGS_COUNT 8

typedef struct
{
    char const * name;
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

typedef struct BufferCopyInfo
{
    daxa_BufferId src_buffer;
    size_t src_offset;
    daxa_BufferId dst_buffer;
    size_t dst_offset;
    size_t size;
};

typedef struct BufferImageCopyInfo
{
    daxa_BufferId buffer;
    size_t buffer_offset;
    daxa_ImageId image;
    VkImageLayout image_layout;
    ImageArraySlice image_slice;
    Offset3D image_offset;
    Extent3D image_extent;
};

typedef struct ImageBufferCopyInfo
{
    daxa_ImageId image;
    VkImageLayout image_layout;
    ImageArraySlice image_slice;
    Offset3D image_offset;
    Extent3D image_extent;
    daxa_BufferId buffer;
    size_t buffer_offset;
};

typedef struct ImageCopyInfo
{
    daxa_ImageId src_image;
    VkImageLayout src_image_layout;
    daxa_ImageId dst_image;
    VkImageLayout dst_image_layout;
    ImageArraySlice src_slice;
    Offset3D src_offset;
    ImageArraySlice dst_slice;
    Offset3D dst_offset;
    Extent3D extent;
};

typedef struct daxa_ImageClearInfo
{
    VkImageLayout dst_image_layout;
    VkClearValue clear_value;
    daxa_ImageId dst_image;
    ImageMipArraySlice dst_slice;
};

struct BufferClearInfo
{
    daxa_BufferId buffer;
    size_t offset;
    size_t size;
    uint32_t clear_value;
};

typedef struct
{
    daxa_ImageViewId image_view;
    VkImageLayout layout;
    VkAttachmentLoadOp load_op;
    VkAttachmentStoreOp store_op;
    VkClearValue clear_value;
} daxa_RenderAttachmentInfo;

typedef struct
{
    daxa_RenderAttachmentInfo const * color_attachments;
    size_t color_attachment_count;
    daxa_RenderAttachmentInfo const * depth_attachment;
    daxa_RenderAttachmentInfo const * stencil_attachment;
    VkRect2D render_area;
} daxa_RenderPassBeginInfo;

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t offset;
} daxa_DispatchIndirectInfo;

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t offset;
    uint32_t draw_count;
    uint32_t stride;
} daxa_DrawMeshTasksIndirectInfo;

typedef struct
{
    daxa_BufferId indirect_buffer;
    size_t offset;
    daxa_BufferId count_buffer;
    size_t count_offset;
    uint32_t max_count;
    uint32_t stride;
} daxa_DrawMeshTasksIndirectCountInfo;

typedef struct
{
    uint32_t vertex_count;
    uint32_t instance_count;
    uint32_t first_vertex;
    uint32_t first_instance;
} daxa_DrawInfo;

typedef struct
{
    uint32_t index_count;
    uint32_t instance_count;
    uint32_t first_index;
    int32_t vertex_offset;
    uint32_t first_instance;
} daxa_DrawIndexedInfo;

typedef struct
{
    daxa_BufferId draw_command_buffer;
    size_t draw_command_buffer_read_offset;
    uint32_t draw_count;
    uint32_t draw_command_stride;
    DAXA_BOOL is_indexed;
} daxa_DrawIndirectInfo;

typedef struct
{
    daxa_BufferId draw_command_buffer;
    size_t draw_command_buffer_read_offset;
    daxa_BufferId draw_count_buffer;
    size_t draw_count_buffer_read_offset;
    uint32_t max_draw_count;
    uint32_t draw_command_stride;
    DAXA_BOOL is_indexed;
} daxa_DrawIndirectCountInfo;

typedef struct
{
    daxa_Event event;
    VkPipelineStageFlags stage;
} daxa_ResetEventsInfo;

typedef struct
{
    daxa_Event events;
    size_t event_count;
} daxa_WaitEventsInfo;

typedef struct
{
    daxa_TimelineQueryPool query_pool;
    VkPipelineStageFlags pipeline_stage;
    uint32_t query_index;
} daxa_WriteTimestampInfo;

typedef struct
{
    daxa_TimelineQueryPool query_pool;
    uint32_t start_index;
    uint32_t count;
} daxa_ResetTimestampsInfo;

typedef struct
{
    float label_color[4];
    char const * name;
} daxa_CommandLabelInfo;

typedef struct
{
    daxa_Event barrier;
    VkPipelineStageFlags stage_masks;
} daxa_ResetEventInfo;

typedef struct
{
    // Binding slot the buffer will be bound to.
    uint32_t slot;
    daxa_BufferId buffer;
    size_t size;
    size_t offset;
} daxa_SetConstantBufferInfo;

typedef struct
{
    float constant_factor;
    float clamp;
    float slope_factor;
} daxa_DepthBiasInfo;

struct daxa_ImplCommandList;
typedef daxa_ImplCommandList * daxa_CommandList;

void 
daxa_cmd_copy_buffer_to_buffer(daxa_CommandList cmd_list, BufferCopyInfo const * info);
void 
daxa_cmd_copy_buffer_to_image(daxa_CommandList cmd_list, BufferImageCopyInfo const * info);
void 
daxa_cmd_copy_image_to_buffer(daxa_CommandList cmd_list, ImageBufferCopyInfo const * info);
void 
daxa_cmd_copy_image_to_image(daxa_CommandList cmd_list, ImageCopyInfo const * info);
void 
daxa_cmd_blit_image_to_image(daxa_CommandList cmd_list, ImageBlitInfo const * info);

void 
daxa_cmd_clear_buffer(daxa_CommandList cmd_list, BufferClearInfo const * info);
void 
daxa_cmd_clear_image(daxa_CommandList cmd_list, VkImageClearInfo const * info);

/// @brief  Successive pipeline barrier calls are combined.
///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
/// @param info parameters.
void 
daxa_cmd_pipeline_barrier(daxa_CommandList cmd_list, daxa_MemoryBarrierInfo const * info);
/// @brief  Successive pipeline barrier calls are combined.
///         As soon as a non-pipeline barrier command is recorded, the currently recorded barriers are flushed with a vkCmdPipelineBarrier2 call.
/// @param info parameters.
void 
daxa_cmd_pipeline_barrier_image_transition(daxa_CommandList cmd_list, daxa_ImageBarrierInfo const * info);
void 
daxa_cmd_signal_event(daxa_CommandList cmd_list, EventSignalInfo const * info);
void 
daxa_cmd_wait_events(daxa_CommandList cmd_list, daxa_EventWaitInfo const * infos, size_t info_count);
void 
daxa_cmd_wait_event(daxa_CommandList cmd_list, daxa_EventWaitInfo const * info);
void 
daxa_cmd_reset_event(daxa_CommandList cmd_list, daxa_ResetEventInfo const * info);

void 
daxa_cmd_push_constant(daxa_CommandList cmd_list, void const * data, uint32_t size, uint32_t offset);
/// @brief  Binds a buffer region to the uniform buffer slot.
///         There are 8 uniform buffer slots (indices range from 0 to 7).
///         The buffer range is user managed, The buffer MUST not be destroyed before the command list is submitted!
///         Changes to these bindings only become visible to commands AFTER a pipeline is bound!
///         This is in stark contrast to OpenGl like bindings wich are visible immediately to all commands after binding.
///         This is deliberate to discourage overuse of uniform buffers over descriptor sets.
///         Set uniform buffer slots are cleared after a pipeline is bound.
///         Before setting another pipeline, they need to be set again.
/// @param info parameters.
void 
daxa_cmd_set_uniform_buffer(daxa_CommandList cmd_list, daxa_SetConstantBufferInfo const * info);
void 
daxa_cmd_set_pipeline(daxa_CommandList cmd_list, daxa_ComputePipeline const * pipeline);
void 
daxa_cmd_set_pipeline(daxa_CommandList cmd_list, daxa_RasterPipeline const * pipeline);
void 
daxa_cmd_dispatch(daxa_CommandList cmd_list, uint32_t x, uint32_t y, uint32_t z);
void 
daxa_cmd_dispatch_indirect(daxa_CommandList cmd_list, daxa_DispatchIndirectInfo const * info);

/// @brief  Destroyes the buffer AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id buffer to be destroyed after command list finishes.
void 
daxa_cmd_destroy_buffer_deferred(daxa_CommandList cmd_list, daxa_BufferId id);
/// @brief  Destroyes the image AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image to be destroyed after command list finishes.
void 
daxa_cmd_destroy_image_deferred(daxa_CommandList cmd_list, daxa_ImageId id);
/// @brief  Destroyes the image view AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image view to be destroyed after command list finishes.
void 
daxa_cmd_destroy_image_view_deferred(daxa_CommandList cmd_list, daxa_ImageViewId id);
/// @brief  Destroyes the sampler AFTER the gpu is finished executing the command list.
///         Useful for large uploads exceeding staging memory pools.
/// @param id image sampler be destroyed after command list finishes.
void 
daxa_cmd_destroy_sampler_deferred(daxa_CommandList cmd_list, daxa_SamplerId id);

/// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
///         Between the begin and end renderpass commands, the renderpass persists and drawcalls can be recorded.
/// @param info parameters.
void 
daxa_cmd_begin_renderpass(daxa_CommandList cmd_list, daxa_RenderPassBeginInfo const * info);
/// @brief  Starts a renderpass scope akin to the dynamic rendering feature in vulkan.
///         Between the begin and end renderpass commands, the renderpass persists and drawcalls can be recorded.
void 
daxa_cmd_end_renderpass(daxa_CommandList cmd_list);
void 
daxa_cmd_set_viewport(daxa_CommandList cmd_list, VkViewportInfo const * info);
void 
daxa_cmd_set_scissor(daxa_CommandList cmd_list, VkRect2D const * info);
void 
daxa_cmd_set_depth_bias(daxa_CommandList cmd_list, daxa_DepthBiasInfo const * info);
void 
daxa_cmd_set_index_buffer(daxa_CommandList cmd_list, daxa_BufferId id, size_t offset, size_t index_type_byte_size);

void 
daxa_cmd_draw(daxa_CommandList cmd_list, daxa_DrawInfo const * info);
void 
daxa_cmd_draw_indexed(daxa_CommandList cmd_list, daxa_DrawIndexedInfo const * info);
void 
daxa_cmd_draw_indirect(daxa_CommandList cmd_list, daxa_DrawIndirectInfo const * info);
void 
daxa_cmd_draw_indirect_count(daxa_CommandList cmd_list, daxa_DrawIndirectCountInfo const * info);
void 
daxa_cmd_draw_mesh_tasks(daxa_CommandList cmd_list, uint32_t x, uint32_t y, uint32_t z);
void 
daxa_cmd_draw_mesh_tasks_indirect(daxa_CommandList cmd_list, daxa_DrawMeshTasksIndirectInfo const * info);
void 
daxa_cmd_draw_mesh_tasks_indirect_count(daxa_CommandList cmd_list, daxa_DrawMeshTasksIndirectCountInfo const * info);

void 
daxa_cmd_write_timestamp(daxa_CommandList cmd_list, daxa_WriteTimestampInfo const * info);
void 
daxa_cmd_reset_timestamps(daxa_CommandList cmd_list, daxa_ResetTimestampsInfo const * info);

void 
daxa_cmd_begin_label(daxa_CommandList cmd_list, daxa_CommandLabelInfo const * info);
void 
daxa_cmd_end_label(daxa_CommandList cmd_list, daxa_CommandLabelInfo label);

void 
daxa_cmd_complete(daxa_CommandList cmd_list, daxa_CommandLabelInfo label);
DAXA_BOOL
daxa_cmd_is_complete(daxa_CommandList cmd_list, daxa_CommandLabelInfo label);
daxa_CommandListInfo const *
daxa_cmd_info(daxa_CommandList cmd_list, daxa_CommandLabelInfo label);

#endif // #ifndef __DAXA_COMMAND_LIST_H__