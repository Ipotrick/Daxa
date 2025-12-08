#ifndef __DAXA_SYNC_H__
#define __DAXA_SYNC_H__

#include <daxa/c/gpu_resources.h>

typedef struct
{
    VkPipelineStageFlags2 stages;
    VkAccessFlags2 access_type;
} daxa_Access;

static daxa_Access const NONE = {.stages = 0, .access_type = 0};

static daxa_Access const DAXA_ACCESS_TOP_OF_PIPE_READ = {.stages = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_DRAW_INDIRECT_READ = {.stages = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_VERTEX_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_TESSELLATION_CONTROL_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_TESSELLATION_EVALUATION_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_GEOMETRY_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_FRAGMENT_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_EARLY_FRAGMENT_TESTS_READ = {.stages = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_LATE_FRAGMENT_TESTS_READ = {.stages = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_COLOR_ATTACHMENT_OUTPUT_READ = {.stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_COMPUTE_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_TRANSFER_READ = {.stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_BOTTOM_OF_PIPE_READ = {.stages = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_HOST_READ = {.stages = VK_PIPELINE_STAGE_2_HOST_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_ALL_GRAPHICS_READ = {.stages = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_READ = {.stages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_COPY_READ = {.stages = VK_PIPELINE_STAGE_2_COPY_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_RESOLVE_READ = {.stages = VK_PIPELINE_STAGE_2_RESOLVE_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_BLIT_READ = {.stages = VK_PIPELINE_STAGE_2_BLIT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_CLEAR_READ = {.stages = VK_PIPELINE_STAGE_2_CLEAR_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_INDEX_INPUT_READ = {.stages = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_PRE_RASTERIZATION_SHADERS_READ = {.stages = VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_TASK_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_MESH_SHADER_READ = {.stages = VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_ACCELERATION_STRUCTURE_BUILD_READ = {.stages = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};
static daxa_Access const DAXA_ACCESS_RAY_TRACING_SHADER_READ = {.stages = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, .access_type = VK_ACCESS_2_MEMORY_READ_BIT};

static daxa_Access const DAXA_ACCESS_TOP_OF_PIPE_WRITE = {.stages = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_DRAW_INDIRECT_WRITE = {.stages = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_VERTEX_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TESSELLATION_CONTROL_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TESSELLATION_EVALUATION_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_GEOMETRY_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_FRAGMENT_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_EARLY_FRAGMENT_TESTS_WRITE = {.stages = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_LATE_FRAGMENT_TESTS_WRITE = {.stages = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_COLOR_ATTACHMENT_OUTPUT_WRITE = {.stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_COMPUTE_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TRANSFER_WRITE = {.stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_BOTTOM_OF_PIPE_WRITE = {.stages = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_HOST_WRITE = {.stages = VK_PIPELINE_STAGE_2_HOST_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_ALL_GRAPHICS_WRITE = {.stages = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_WRITE = {.stages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_COPY_WRITE = {.stages = VK_PIPELINE_STAGE_2_COPY_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_RESOLVE_WRITE = {.stages = VK_PIPELINE_STAGE_2_RESOLVE_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_BLIT_WRITE = {.stages = VK_PIPELINE_STAGE_2_BLIT_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_CLEAR_WRITE = {.stages = VK_PIPELINE_STAGE_2_CLEAR_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_INDEX_INPUT_WRITE = {.stages = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_PRE_RASTERIZATION_SHADERS_WRITE = {.stages = VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TASK_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_MESH_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_ACCELERATION_STRUCTURE_BUILD_WRITE = {.stages = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_RAY_TRACING_SHADER_WRITE = {.stages = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, .access_type = VK_ACCESS_2_MEMORY_WRITE_BIT};

static daxa_Access const DAXA_ACCESS_TOP_OF_PIPE_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_DRAW_INDIRECT_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_VERTEX_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TESSELLATION_CONTROL_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TESSELLATION_EVALUATION_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_GEOMETRY_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_FRAGMENT_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_EARLY_FRAGMENT_TESTS_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_LATE_FRAGMENT_TESTS_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_COLOR_ATTACHMENT_OUTPUT_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_COMPUTE_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TRANSFER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_BOTTOM_OF_PIPE_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_HOST_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_HOST_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_ALL_GRAPHICS_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_COPY_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_COPY_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_RESOLVE_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_RESOLVE_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_BLIT_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_BLIT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_CLEAR_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_CLEAR_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_INDEX_INPUT_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_PRE_RASTERIZATION_SHADERS_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_TASK_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_MESH_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_ACCELERATION_STRUCTURE_BUILD_READ_WRITE = {.stages = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};
static daxa_Access const DAXA_ACCESS_RAY_TRACING_SHADER_READ_WRITE = {.stages = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, .access_type = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT};

typedef struct
{
    daxa_Access src_access;
    daxa_Access dst_access;
} daxa_BarrierInfo;

static const daxa_BarrierInfo DAXA_DEFAULT_BARRIER_INFO = DAXA_ZERO_INIT;

#if !DAXA_REMOVE_DEPRECATED
/* deprecated("Use daxa_BarrierInfo instead; API:3.2") */
#define daxa_MemoryBarrierInfo daxa_BarrierInfo
#endif

#if !DAXA_REMOVE_DEPRECATED

/* deprecated("Use ImageBarrierInfo instead; API:3.2") */ typedef struct
{
    daxa_Access src_access;
    daxa_Access dst_access;
    daxa_ImageLayout src_layout;
    daxa_ImageLayout dst_layout;
    /* deprecated("Ignored parameter, whole image will be transitioned; API:3.2") */ daxa_ImageMipArraySlice image_slice;
    daxa_ImageId image_id;
} daxa_ImageMemoryBarrierInfo;

static const daxa_ImageMemoryBarrierInfo DAXA_DEFAULT_BARRIER_IMAGE_TRANSITION_INFO = DAXA_ZERO_INIT;

#endif

typedef enum
{
    DAXA_IMAGE_LAYOUT_OPERATION_NONE = 0,
    DAXA_IMAGE_LAYOUT_OPERATION_TO_GENERAL = 1,
    DAXA_IMAGE_LAYOUT_OPERATION_TO_PRESENT_SRC = 2,
} daxa_ImageLayoutOperation;

typedef struct
{
    daxa_Access src_access;
    daxa_Access dst_access;
    daxa_ImageId image_id;
    daxa_ImageLayoutOperation layout_operation;
} daxa_ImageBarrierInfo;

static const daxa_ImageBarrierInfo DAXA_DEFAULT_IMAGE_BARRIER_INFO = DAXA_ZERO_INIT;

typedef struct
{
    daxa_SmallString name;
} daxa_BinarySemaphoreInfo;

static const daxa_BinarySemaphoreInfo DAXA_DEFAULT_BINARY_SEMAPHORE_INFO = DAXA_ZERO_INIT;

DAXA_EXPORT daxa_BinarySemaphoreInfo const *
daxa_binary_semaphore_info(daxa_BinarySemaphore binary_semaphore);

DAXA_EXPORT VkSemaphore
daxa_binary_semaphore_get_vk_semaphore(daxa_BinarySemaphore binary_semaphore);

DAXA_EXPORT uint64_t
daxa_binary_semaphore_inc_refcnt(daxa_BinarySemaphore binary_semaphore);
DAXA_EXPORT uint64_t
daxa_binary_semaphore_dec_refcnt(daxa_BinarySemaphore binary_semaphore);

typedef struct
{
    uint64_t initial_value;
    daxa_SmallString name;
} daxa_TimelineSemaphoreInfo;

static const daxa_TimelineSemaphoreInfo DAXA_DEFAULT_TIMELINE_SEMAPHORE_INFO = DAXA_ZERO_INIT;

DAXA_EXPORT daxa_TimelineSemaphoreInfo const *
daxa_timeline_semaphore_info(daxa_TimelineSemaphore timeline_semaphore);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_timeline_semaphore_get_value(daxa_TimelineSemaphore timeline_semaphore, uint64_t * out_value);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_timeline_semaphore_set_value(daxa_TimelineSemaphore timeline_semaphore, uint64_t value);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_timeline_semaphore_wait_for_value(daxa_TimelineSemaphore timeline_semaphore, uint64_t value, uint64_t timeout);

DAXA_EXPORT VkSemaphore
daxa_timeline_semaphore_get_vk_semaphore(daxa_TimelineSemaphore timeline_semaphore);

DAXA_EXPORT uint64_t
daxa_timeline_semaphore_inc_refcnt(daxa_TimelineSemaphore timeline_semaphore);
DAXA_EXPORT uint64_t
daxa_timeline_semaphore_dec_refcnt(daxa_TimelineSemaphore timeline_semaphore);

typedef struct
{
    daxa_SmallString name;
} daxa_EventInfo;

static const daxa_EventInfo DAXA_DEFAULT_EVENT_INFO = DAXA_ZERO_INIT;

typedef struct
{
    daxa_BarrierInfo const * barriers;
    uint64_t barrier_count;
    daxa_ImageBarrierInfo const * image_barriers;
    uint64_t image_barrier_count;
    daxa_Event * event;
} daxa_EventSignalInfo;

static const daxa_EventSignalInfo DAXA_DEFAULT_EVENT_SIGNAL_INFO = DAXA_ZERO_INIT;

typedef daxa_EventSignalInfo daxa_EventWaitInfo;

DAXA_EXPORT daxa_EventInfo const *
daxa_event_info(daxa_Event event);

DAXA_EXPORT uint64_t
daxa_event_inc_refcnt(daxa_Event event);
DAXA_EXPORT uint64_t
daxa_event_dec_refcnt(daxa_Event event);

typedef struct
{
    daxa_TimelineSemaphore semaphore;
    uint64_t value;
} daxa_TimelinePair;

static const daxa_TimelinePair DAXA_DEFAULT_TIMELINE_PAIR = DAXA_ZERO_INIT;

#endif // #if __DAXA_SYNC_H__
