#ifndef __DAXA_SYNC_H__
#define __DAXA_SYNC_H__

#include "types.h"

typedef struct
{
    VkPipelineStageFlags stages;
    VkAccessFlags access_type;
} daxa_Access;

extern const daxa_Access NONE;

extern const daxa_Access DAXA_ACCESS_TOP_OF_PIPE_READ;
extern const daxa_Access DAXA_ACCESS_DRAW_INDIRECT_READ;
extern const daxa_Access DAXA_ACCESS_VERTEX_SHADER_READ;
extern const daxa_Access DAXA_ACCESS_TESSELLATION_CONTROL_SHADER_READ;
extern const daxa_Access DAXA_ACCESS_TESSELLATION_EVALUATION_SHADER_READ;
extern const daxa_Access DAXA_ACCESS_GEOMETRY_SHADER_READ;
extern const daxa_Access DAXA_ACCESS_FRAGMENT_SHADER_READ;
extern const daxa_Access DAXA_ACCESS_EARLY_FRAGMENT_TESTS_READ;
extern const daxa_Access DAXA_ACCESS_LATE_FRAGMENT_TESTS_READ;
extern const daxa_Access DAXA_ACCESS_COLOR_ATTACHMENT_OUTPUT_READ;
extern const daxa_Access DAXA_ACCESS_COMPUTE_SHADER_READ;
extern const daxa_Access DAXA_ACCESS_TRANSFER_READ;
extern const daxa_Access DAXA_ACCESS_BOTTOM_OF_PIPE_READ;
extern const daxa_Access DAXA_ACCESS_HOST_READ;
extern const daxa_Access DAXA_ACCESS_ALL_GRAPHICS_READ;
extern const daxa_Access DAXA_ACCESS_READ;
extern const daxa_Access DAXA_ACCESS_COPY_READ;
extern const daxa_Access DAXA_ACCESS_RESOLVE_READ;
extern const daxa_Access DAXA_ACCESS_BLIT_READ;
extern const daxa_Access DAXA_ACCESS_CLEAR_READ;
extern const daxa_Access DAXA_ACCESS_INDEX_INPUT_READ;
extern const daxa_Access DAXA_ACCESS_PRE_RASTERIZATION_SHADERS_READ;
extern const daxa_Access DAXA_ACCESS_TASK_SHADER_READ;
extern const daxa_Access DAXA_ACCESS_MESH_SHADER_READ;

extern const daxa_Access DAXA_ACCESS_TOP_OF_PIPE_WRITE;
extern const daxa_Access DAXA_ACCESS_DRAW_INDIRECT_WRITE;
extern const daxa_Access DAXA_ACCESS_VERTEX_SHADER_WRITE;
extern const daxa_Access DAXA_ACCESS_TESSELLATION_CONTROL_SHADER_WRITE;
extern const daxa_Access DAXA_ACCESS_TESSELLATION_EVALUATION_SHADER_WRITE;
extern const daxa_Access DAXA_ACCESS_GEOMETRY_SHADER_WRITE;
extern const daxa_Access DAXA_ACCESS_FRAGMENT_SHADER_WRITE;
extern const daxa_Access DAXA_ACCESS_EARLY_FRAGMENT_TESTS_WRITE;
extern const daxa_Access DAXA_ACCESS_LATE_FRAGMENT_TESTS_WRITE;
extern const daxa_Access DAXA_ACCESS_COLOR_ATTACHMENT_OUTPUT_WRITE;
extern const daxa_Access DAXA_ACCESS_COMPUTE_SHADER_WRITE;
extern const daxa_Access DAXA_ACCESS_TRANSFER_WRITE;
extern const daxa_Access DAXA_ACCESS_BOTTOM_OF_PIPE_WRITE;
extern const daxa_Access DAXA_ACCESS_HOST_WRITE;
extern const daxa_Access DAXA_ACCESS_ALL_GRAPHICS_WRITE;
extern const daxa_Access DAXA_ACCESS_WRITE;
extern const daxa_Access DAXA_ACCESS_COPY_WRITE;
extern const daxa_Access DAXA_ACCESS_RESOLVE_WRITE;
extern const daxa_Access DAXA_ACCESS_BLIT_WRITE;
extern const daxa_Access DAXA_ACCESS_CLEAR_WRITE;
extern const daxa_Access DAXA_ACCESS_INDEX_INPUT_WRITE;
extern const daxa_Access DAXA_ACCESS_PRE_RASTERIZATION_SHADERS_WRITE;
extern const daxa_Access DAXA_ACCESS_TASK_SHADER_WRITE;
extern const daxa_Access DAXA_ACCESS_MESH_SHADER_WRITE;

extern const daxa_Access DAXA_ACCESS_TOP_OF_PIPE_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_DRAW_INDIRECT_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_VERTEX_SHADER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_TESSELLATION_CONTROL_SHADER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_TESSELLATION_EVALUATION_SHADER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_GEOMETRY_SHADER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_FRAGMENT_SHADER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_EARLY_FRAGMENT_TESTS_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_LATE_FRAGMENT_TESTS_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_COLOR_ATTACHMENT_OUTPUT_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_COMPUTE_SHADER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_TRANSFER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_BOTTOM_OF_PIPE_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_HOST_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_ALL_GRAPHICS_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_COPY_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_RESOLVE_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_BLIT_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_CLEAR_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_INDEX_INPUT_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_PRE_RASTERIZATION_SHADERS_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_TASK_SHADER_READ_WRITE;
extern const daxa_Access DAXA_ACCESS_MESH_SHADER_READ_WRITE;

typedef struct 
{
    daxa_Access src_access;
    daxa_Access dst_access;
} daxa_MemoryBarrierInfo;

typedef struct 
{
    daxa_Access src_access;
    daxa_Access dst_access;
    daxa_ImageLayout src_layout;
    daxa_ImageLayout dst_layout;
    daxa_ImageMipArraySlice image_slice;
    daxa_ImageId image_id;
} daxa_ImageMemoryBarrierInfo;

struct daxa_ImplBinarySemaphore;
typedef struct daxa_ImplBinarySemaphore * daxa_BinarySemaphore;

typedef struct
{
    char const * name;
} daxa_BinarySemaphoreInfo;

daxa_BinarySemaphoreInfo const *
daxa_binary_semaphore_info(daxa_BinarySemaphore binary_semaphore);

struct daxa_ImplTimelineSemaphore;
typedef struct daxa_ImplTimelineSemaphore * daxa_TimelineSemaphore;

typedef struct
{
    uint64_t initial_value;
    char const * name;
} daxa_TimelineSemaphoreInfo;

daxa_TimelineSemaphoreInfo const *
daxa_timeline_semaphore_info(daxa_TimelineSemaphore timeline_semaphore);

uint64_t
daxa_timeline_semaphore_get_value(daxa_TimelineSemaphore timeline_semaphore);

void
daxa_timeline_semaphore_set_value(daxa_TimelineSemaphore timeline_semaphore, uint64_t value);

daxa_Result
daxa_timeline_semaphore_wait_for_value(daxa_TimelineSemaphore timeline_semaphore, uint64_t value, uint64_t timeout);

struct daxa_ImplEvent;
typedef struct daxa_ImplEvent * daxa_Event;

typedef struct
{
    char const * name;
} daxa_EventInfo;

daxa_Event const *
daxa_event_info(daxa_Event event);

#endif // #if __DAXA_SYNC_H__