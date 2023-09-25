#ifndef __DAXA_DEVICE_H__
#define __DAXA_DEVICE_H__

#include <daxa/c/core.h>
#include <daxa/c/gpu_resources.h>
#include <daxa/c/pipeline.h>
#include <daxa/c/swapchain.h>
#include <daxa/c/command_list.h>
#include <daxa/c/semaphore.h>
#include <daxa/c/split_barrier.h>
#include <daxa/c/timeline_query.h>
#include <daxa/c/memory_block.h>

typedef enum
{
    DAXA_DEVICE_TYPE_OTHER = 0,
    DAXA_DEVICE_TYPE_INTEGRATED_GPU = 1,
    DAXA_DEVICE_TYPE_DISCRETE_GPU = 2,
    DAXA_DEVICE_TYPE_VIRTUAL_GPU = 3,
    DAXA_DEVICE_TYPE_CPU = 4,
    DAXA_DEVICE_TYPE_MAX_ENUM = 0x7fffffff,
} daxa_DeviceType;

int32_t
daxa_default_device_score(VkPhysicalDeviceProperties const * properties, VkPhysicalDeviceLimits const * limits)
{
    int32_t score = 1;
    switch (properties->deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU : score += 10000; break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU : score += 1000; break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU : score += 100; break;
    default: break;
    }
    return score;
}

typedef struct
{
    int32_t(*)(VkPhysicalDeviceProperties const * properties, VkPhysicalDeviceLimits const * limits) selector;
    daxa_DeviceFlags flags;
    uint32_t max_allowed_images;
    uint32_t max_allowed_buffers;
    uint32_t max_allowed_samplers;
    char const * name;
} daxa_DeviceInfo;

typedef struct
{
    daxa_PipelineStageFlags wait_stages;
    daxa_CommandList const * cmd_lists;
    uint64_t cmd_list_count;
    daxa_BinarySemaphore const * wait_binary_semaphores;
    uint64_t wait_binary_semaphore_count;
    daxa_BinarySemaphore const * signal_binary_semaphores;
    uint64_t signal_binary_semaphore_count;
    daxa_BinarySemaphore const * wait_timeline_semaphores;
    uint64_t wait_timeline_semaphore_count;
    daxa_BinarySemaphore const * signal_timeline_semaphores;
    uint64_t signal_timeline_semaphoreCount;    
} daxa_CommandSubmitInfo;

typedef struct 
{
    daxa_BinarySemaphore const * wait_binary_semaphores;
    uint64_t wait_binary_semaphore_count;
    daxa_Swapchain swapchain;
} PresentInfo;

struct daxa_ImplDevice;
typedef struct daxa_ImplDevice * daxa_Device;

VkMemoryRequirements 
daxa_dvc_buffer_memory_requirements(daxa_Device device, daxa_BufferInfo const * info);
VkMemoryRequirements 
daxa_dvc_image_memory_requirements(daxa_Device device, daxa_ImageInfo const * info);
daxa_Result 
daxa_dvc_create_memory(daxa_Device device, daxa_MemoryBlockInfo const * info, daxa_MemoryBlock * out_memory_block);

daxa_Result
daxa_dvc_create_buffer(daxa_Device device, daxa_BufferInfo const * info, daxa_BufferId * out_id);
daxa_Result 
daxa_dvc_create_image(daxa_Device device, daxa_ImageInfo const * info, daxa_ImageId * out_id);
daxa_Result
daxa_dvc_create_image_view(daxa_Device device, daxa_ImageViewInfo const * info, daxa_ImageViewId * out_id);
daxa_Result
daxa_dvc_create_sampler(daxa_Device device, daxa_SamplerInfo const * info, daxa_SamplerId * out_id);

daxa_Result 
daxa_dvc_destroy_buffer(daxa_Device device, daxa_BufferId id);
daxa_Result 
daxa_dvc_destroy_image(daxa_Device device, daxa_ImageId id);
daxa_Result 
daxa_dvc_destroy_image_view(daxa_Device device, daxa_ImageViewId id);
daxa_Result 
daxa_dvc_destroy_sampler(daxa_Device device, daxa_SamplerId id);

daxa_Result
daxa_dvc_info_buffer(daxa_Device device, daxa_BufferId id, daxa_BufferInfo * out_info);
daxa_Result
daxa_dvc_info_image(daxa_Device device, daxa_ImageId id, daxa_ImageInfo * out_info);
daxa_Result
daxa_dvc_info_image_view(daxa_Device device, daxa_ImageViewId id, daxa_ImageViewInfo * out_info);
daxa_Result
daxa_dvc_info_sampler(daxa_Device device, daxa_SamplerId id, daxa_SamplerInfo * out_info);

DAXA_BOOL
daxa_dvc_is_buffer_valid(daxa_BufferId id);
DAXA_BOOL
daxa_dvc_is_buffer_valid(daxa_ImageId id);
DAXA_BOOL
daxa_dvc_is_buffer_valid(daxa_ImageViewId id);
DAXA_BOOL
daxa_dvc_is_buffer_valid(daxa_SamplerId id);

daxa_Result
daxa_dvc_buffer_device_address(daxa_BufferId id, daxa_BufferDeviceAddress * out_bda);
daxa_Result
daxa_dvc_buffer_host_address(daxa_BufferId id, void** out_ptr);

daxa_Result
daxa_dvc_create_raster_pipeline(daxa_RasterPipelineInfo const * info, daxa_RasterPipeline * out_pipeline);
daxa_Result
daxa_dvc_create_compute_pipeline(daxa_ComputePipelineInfo const * info, daxa_ComputePipeline * out_pipeline);

daxa_Result
daxa_dvc_create_swapchain(daxa_SwapchainInfo const * info, daxa_Swapchain * out_swapchain);
daxa_Result
daxa_dvc_create_command_list(daxa_CommandListInfoInfo const * info, daxa_CommandListInfo * out_command_list);
daxa_Result
daxa_dvc_create_binary_semaphore(daxa_BinarySemaphoreInfo const * info, daxa_BinarySemaphore * out_binary_semaphore);
daxa_Result
daxa_dvc_create_timeline_semaphore(daxa_TimelineSemaphoreInfo const * info, daxa_TimelineSemaphore * out_timeline_semaphore);
daxa_Result
daxa_dvc_create_event(daxa_EventInfo const * info, daxa_Event * out_event);
daxa_Result
daxa_dvc_create_timeline_query_pool(daxa_TimelineQueryPoolInfo const * info, daxa_TimelineQueryPool * out_timeline_query_pool);

daxa_DeviceInfo const *
daxa_dvc_info(daxa_Device device);
VkDevice
daxa_dvc_get_vk_device(daxa_Device device);

void
daxa_dvc_submit(daxa_Device device, daxa_CommandSubmitInfo const * info);
void 
daxa_dvc_present(daxa_Device device, daxa_PresentInfo const * info);
void
daxa_dvc_wait_idle(daxa_Device device);
void
daxa_dvc_collect_garbage(daxa_Device device);

#endif // #ifndef __DAXA_DEVICE_H__