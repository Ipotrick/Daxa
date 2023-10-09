#ifndef __DAXA_DEVICE_H__
#define __DAXA_DEVICE_H__

#include <daxa/c/types.h>
#include <daxa/c/sync.h>
#include <daxa/c/gpu_resources.h>
#include <daxa/c/pipeline.h>
#include <daxa/c/command_list.h>
#include <daxa/c/swapchain.h>
#include <daxa/c/sync.h>

typedef enum
{
    DAXA_DEVICE_TYPE_OTHER = 0,
    DAXA_DEVICE_TYPE_INTEGRATED_GPU = 1,
    DAXA_DEVICE_TYPE_DISCRETE_GPU = 2,
    DAXA_DEVICE_TYPE_VIRTUAL_GPU = 3,
    DAXA_DEVICE_TYPE_CPU = 4,
    DAXA_DEVICE_TYPE_MAX_ENUM = 0x7fffffff,
} daxa_DeviceType;

DAXA_EXPORT int32_t
daxa_default_device_score(VkPhysicalDeviceProperties const * properties);

typedef enum
{
    DAXA_DEVICE_FLAG_BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT = 0x1 << 0,
    DAXA_DEVICE_FLAG_CONSERVATIVE_RASTERIZATION = 0x1 << 1,
    DAXA_DEVICE_FLAG_MESH_SHADER_BIT = 0x1 << 2,
    DAXA_DEVICE_FLAG_SHADER_ATOMIC64 = 0x1 << 3,
    DAXA_DEVICE_FLAG_IMAGE_ATOMIC64 = 0x1 << 4,
    DAXA_DEVICE_FLAG_VK_MEMORY_MODEL = 0x1 << 5,
} daxa_DeviceFlagBits;

typedef uint64_t daxa_DeviceFlags;

typedef struct
{
    int32_t (*selector)(VkPhysicalDeviceProperties const * properties);
    daxa_DeviceFlags flags;
    uint32_t max_allowed_images;
    uint32_t max_allowed_buffers;
    uint32_t max_allowed_samplers;
    daxa_StringView name;
} daxa_DeviceInfo;

static const daxa_DeviceInfo DAXA_DEFAULT_DEVICE_INFO = {
    .selector = &daxa_default_device_score,
    .flags = DAXA_DEVICE_FLAG_BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT,
    .max_allowed_images = 10000,
    .max_allowed_buffers = 10000,
    .max_allowed_samplers = 10000,
    .name = {},
};

typedef struct 
{
    daxa_TimelineSemaphore semaphore;
    uint64_t value;
} daxa_TimelinePair;

typedef struct
{
    VkPipelineStageFlags wait_stages;
    daxa_CommandList const * command_lists;
    uint64_t command_list_count;
    daxa_BinarySemaphore const * wait_binary_semaphores;
    uint64_t wait_binary_semaphore_count;
    daxa_BinarySemaphore const * signal_binary_semaphores;
    uint64_t signal_binary_semaphore_count;
    daxa_TimelinePair const * wait_timeline_semaphores;
    uint64_t wait_timeline_semaphore_count;
    daxa_TimelinePair const * signal_timeline_semaphores;
    uint64_t signal_timeline_semaphore_count;
} daxa_CommandSubmitInfo;

static const daxa_CommandSubmitInfo DAXA_DEFAULT_COMMAND_SUBMIT_INFO = {};

typedef struct
{
    daxa_BinarySemaphore const * wait_binary_semaphores;
    uint64_t wait_binary_semaphore_count;
    daxa_Swapchain swapchain;
} daxa_PresentInfo;

static const daxa_PresentInfo DAXA_DEFAULT_PRESENT_INFO = {};

// Returns previous ref count.
DAXA_EXPORT uint64_t
daxa_dvc_inc_refcnt(daxa_Device device);
// Returns previous ref count.
DAXA_EXPORT uint64_t
daxa_dvc_dec_refcnt(daxa_Device device);

DAXA_EXPORT VkMemoryRequirements
daxa_dvc_buffer_memory_requirements(daxa_Device device, daxa_BufferInfo const * info);
DAXA_EXPORT VkMemoryRequirements
daxa_dvc_image_memory_requirements(daxa_Device device, daxa_ImageInfo const * info);
DAXA_EXPORT daxa_Result
daxa_dvc_create_memory(daxa_Device device, daxa_MemoryBlockInfo const * info, daxa_MemoryBlock * out_memory_block);

DAXA_EXPORT daxa_Result
daxa_dvc_create_buffer(daxa_Device device, daxa_BufferInfo const * info, daxa_BufferId * out_id);
DAXA_EXPORT daxa_Result
daxa_dvc_create_image(daxa_Device device, daxa_ImageInfo const * info, daxa_ImageId * out_id);
DAXA_EXPORT daxa_Result
daxa_dvc_create_image_view(daxa_Device device, daxa_ImageViewInfo const * info, daxa_ImageViewId * out_id);
DAXA_EXPORT daxa_Result
daxa_dvc_create_sampler(daxa_Device device, daxa_SamplerInfo const * info, daxa_SamplerId * out_id);

DAXA_EXPORT uint64_t
daxa_dvc_inc_refcnt_buffer(daxa_Device device, daxa_BufferId buffer);
DAXA_EXPORT uint64_t
daxa_dvc_inc_refcnt_image(daxa_Device device, daxa_ImageId image);
DAXA_EXPORT uint64_t
daxa_dvc_inc_refcnt_image_view(daxa_Device device, daxa_ImageViewId id);
DAXA_EXPORT uint64_t
daxa_dvc_inc_refcnt_sampler(daxa_Device device, daxa_SamplerId sampler);

DAXA_EXPORT uint64_t
daxa_dvc_dec_refcnt_buffer(daxa_Device device, daxa_BufferId buffer);
DAXA_EXPORT uint64_t
daxa_dvc_dec_refcnt_image(daxa_Device device, daxa_ImageId image);
DAXA_EXPORT uint64_t
daxa_dvc_dec_refcnt_image_view(daxa_Device device, daxa_ImageViewId id);
DAXA_EXPORT uint64_t
daxa_dvc_dec_refcnt_sampler(daxa_Device device, daxa_SamplerId sampler);

DAXA_EXPORT daxa_BufferInfo const *
daxa_dvc_info_buffer(daxa_Device device, daxa_BufferId buffer);
DAXA_EXPORT daxa_ImageInfo const *
daxa_dvc_info_image(daxa_Device device, daxa_ImageId image);
DAXA_EXPORT daxa_ImageViewInfo const *
daxa_dvc_info_image_view(daxa_Device device, daxa_ImageViewId id);
DAXA_EXPORT daxa_SamplerInfo const *
daxa_dvc_info_sampler(daxa_Device device, daxa_SamplerId sampler);

DAXA_EXPORT daxa_Bool8
daxa_dvc_is_buffer_valid(daxa_Device device, daxa_BufferId buffer);
DAXA_EXPORT daxa_Bool8
daxa_dvc_is_image_valid(daxa_Device device, daxa_ImageId image);
DAXA_EXPORT daxa_Bool8
daxa_dvc_is_image_view_valid(daxa_Device device, daxa_ImageViewId image_view);
DAXA_EXPORT daxa_Bool8
daxa_dvc_is_sampler_valid(daxa_Device device, daxa_SamplerId sampler);

DAXA_EXPORT VkBuffer
daxa_dvc_get_vk_buffer(daxa_Device device, daxa_BufferId buffer);
DAXA_EXPORT VkImage
daxa_dvc_get_vk_image(daxa_Device device, daxa_ImageId image);
DAXA_EXPORT VkImageView
daxa_dvc_get_default_vk_image_view(daxa_Device device, daxa_ImageId image);
DAXA_EXPORT VkImageView
daxa_dvc_get_vk_image_view(daxa_Device device, daxa_ImageViewId image_view);
DAXA_EXPORT VkSampler
daxa_dvc_get_vk_sampler(daxa_Device device, daxa_SamplerId sampler);

DAXA_EXPORT daxa_Result
daxa_dvc_buffer_device_address(daxa_Device device, daxa_BufferId buffer, daxa_BufferDeviceAddress * out_bda);
DAXA_EXPORT daxa_Result
daxa_dvc_buffer_host_address(daxa_Device device, daxa_BufferId buffer, void ** out_ptr);

DAXA_EXPORT daxa_Result
daxa_dvc_create_raster_pipeline(daxa_Device device, daxa_RasterPipelineInfo const * info, daxa_RasterPipeline * out_pipeline);
DAXA_EXPORT daxa_Result
daxa_dvc_create_compute_pipeline(daxa_Device device, daxa_ComputePipelineInfo const * info, daxa_ComputePipeline * out_pipeline);
DAXA_EXPORT daxa_Result
daxa_dvc_create_swapchain(daxa_Device device, daxa_SwapchainInfo const * info, daxa_Swapchain * out_swapchain);
DAXA_EXPORT daxa_Result
daxa_dvc_create_command_list(daxa_Device device, daxa_CommandListInfo const * info, daxa_CommandList * out_command_list);
DAXA_EXPORT daxa_Result
daxa_dvc_create_binary_semaphore(daxa_Device device, daxa_BinarySemaphoreInfo const * info, daxa_BinarySemaphore * out_binary_semaphore);
DAXA_EXPORT daxa_Result
daxa_dvc_create_timeline_semaphore(daxa_Device device, daxa_TimelineSemaphoreInfo const * info, daxa_TimelineSemaphore * out_timeline_semaphore);
DAXA_EXPORT daxa_Result
daxa_dvc_create_event(daxa_Device device, daxa_EventInfo const * info, daxa_Event * out_event);
DAXA_EXPORT daxa_Result
daxa_dvc_create_timeline_query_pool(daxa_Device device, daxa_TimelineQueryPoolInfo const * info, daxa_TimelineQueryPool * out_timeline_query_pool);

DAXA_EXPORT daxa_DeviceInfo const *
daxa_dvc_info(daxa_Device device);
DAXA_EXPORT VkDevice
daxa_dvc_get_vk_device(daxa_Device device);

DAXA_EXPORT daxa_Result
daxa_dvc_wait_idle(daxa_Device device);
DAXA_EXPORT daxa_Result
daxa_dvc_submit(daxa_Device device, daxa_CommandSubmitInfo const * info);
DAXA_EXPORT daxa_Result
daxa_dvc_present(daxa_Device device, daxa_PresentInfo const * info);
DAXA_EXPORT daxa_Result
daxa_dvc_collect_garbage(daxa_Device device);

#endif // #ifndef __DAXA_DEVICE_H__
