#ifndef __DAXA_DEVICE_H__
#define __DAXA_DEVICE_H__

#include "daxa/c/types.h"
#include "daxa/c/sync.h"
#include "daxa/c/gpu_resources.h"
#include "daxa/c/pipeline.h"
#include "daxa/c/command_list.h"

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
daxa_default_device_score(VkPhysicalDeviceProperties const * properties, VkPhysicalDeviceLimits const * limits);

typedef enum
{
    DAXA_DEVICE_FLAG_BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT = 0x1,
    DAXA_DEVICE_FLAG_CONSERVATIVE_RASTERIZATION = 0x2,
    DAXA_DEVICE_FLAG_MESH_SHADER_BIT = 0x4,
} daxa_DeviceFlagBits;

typedef uint64_t daxa_DeviceFlags;

typedef struct
{
    int32_t (*selector)(VkPhysicalDeviceProperties const * properties, VkPhysicalDeviceLimits const * limits);
    daxa_DeviceFlags flags;
    uint32_t max_allowed_images;
    uint32_t max_allowed_buffers;
    uint32_t max_allowed_samplers;
    char const * name;
} daxa_DeviceInfo;

typedef struct
{
    VkPipelineStageFlags wait_stages;
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
daxa_dvc_destroy_buffer(daxa_Device device, daxa_BufferId buffer);
daxa_Result
daxa_dvc_destroy_image(daxa_Device device, daxa_ImageId image);
daxa_Result
daxa_dvc_destroy_image_view(daxa_Device device, daxa_ImageViewId id);
daxa_Result
daxa_dvc_destroy_sampler(daxa_Device device, daxa_SamplerId sampler);

daxa_Result
daxa_dvc_info_buffer(daxa_Device device, daxa_BufferId buffer, daxa_BufferInfo * out_info);
daxa_Result
daxa_dvc_info_image(daxa_Device device, daxa_ImageId image, daxa_ImageInfo * out_info);
daxa_Result
daxa_dvc_info_image_view(daxa_Device device, daxa_ImageViewId id, daxa_ImageViewInfo * out_info);
daxa_Result
daxa_dvc_info_sampler(daxa_Device device, daxa_SamplerId sampler, daxa_SamplerInfo * out_info);

VkBool32
daxa_dvc_is_buffer_valid(daxa_Device device, daxa_BufferId buffer);
VkBool32
daxa_dvc_is_buffer_valid(daxa_Device device, daxa_ImageId image);
VkBool32
daxa_dvc_is_buffer_valid(daxa_Device device, daxa_ImageViewId image_view);
VkBool32
daxa_dvc_is_buffer_valid(daxa_Device device, daxa_SamplerId sampler);

daxa_Result
daxa_dvc_get_vk_buffer(daxa_Device device, daxa_BufferId buffer, VkBuffer * out_vk_buffer);
daxa_Result
daxa_dvc_get_vk_image(daxa_Device device, daxa_ImageId image, VkImage * out_vk_image);
daxa_Result
daxa_dvc_get_default_vk_image_view(daxa_Device device, daxa_ImageId image, VkImageView * out_vk_image_view);
daxa_Result
daxa_dvc_get_vk_image_view(daxa_Device device, daxa_ImageViewId image_view, VkImageView * out_vk_image_view);
daxa_Result
daxa_dvc_get_vk_sampler(daxa_Device device, daxa_SamplerId sampler, VkSampler * out_vk_sampler);

daxa_Result
daxa_dvc_buffer_device_address(daxa_Device device, daxa_BufferId buffer, daxa_BufferDeviceAddress * out_bda);
daxa_Result
daxa_dvc_buffer_host_address(daxa_Device device, daxa_BufferId buffer, void ** out_ptr);

daxa_Result
daxa_dvc_create_raster_pipeline(daxa_Device device, daxa_RasterPipelineInfo const * info, daxa_RasterPipeline * out_pipeline);
daxa_Result
daxa_dvc_create_compute_pipeline(daxa_Device device, daxa_ComputePipelineInfo const * info, daxa_ComputePipeline * out_pipeline);
daxa_Result
daxa_dvc_create_swapchain(daxa_Device device, daxa_SwapchainInfo const * info, daxa_Swapchain * out_swapchain);
daxa_Result
daxa_dvc_create_command_list(daxa_Device device, daxa_CommandListInfoInfo const * info, daxa_CommandListInfo * out_command_list);
daxa_Result
daxa_dvc_create_binary_semaphore(daxa_Device device, daxa_BinarySemaphoreInfo const * info, daxa_BinarySemaphore * out_binary_semaphore);
daxa_Result
daxa_dvc_create_timeline_semaphore(daxa_Device device, daxa_TimelineSemaphoreInfo const * info, daxa_TimelineSemaphore * out_timeline_semaphore);
daxa_Result
daxa_dvc_create_event(daxa_Device device, daxa_EventInfo const * info, daxa_Event * out_event);
daxa_Result
daxa_dvc_create_timeline_query_pool(daxa_Device device, daxa_TimelineQueryPoolInfo const * info, daxa_TimelineQueryPool * out_timeline_query_pool);

daxa_Result
daxa_dvc_destroy_raster_pipeline(daxa_Device device, daxa_RasterPipeline pipeline);
daxa_Result
daxa_dvc_destroy_compute_pipeline(daxa_Device device, daxa_ComputePipeline pipeline);
daxa_Result
daxa_dvc_destroy_swapchain(daxa_Device device, daxa_Swapchain swapchain);
daxa_Result
daxa_dvc_destroy_command_list(daxa_Device device, daxa_CommandListInfo command_list);
daxa_Result
daxa_dvc_destroy_binary_semaphore(daxa_Device device, daxa_BinarySemaphore binary_semaphore);
daxa_Result
daxa_dvc_destroy_timeline_semaphore(daxa_Device device, daxa_TimelineSemaphore timeline_semaphore);
daxa_Result
daxa_dvc_destroy_event(daxa_Device device, daxa_Event event);
daxa_Result
daxa_dvc_destroy_timeline_query_pool(daxa_Device device, daxa_TimelineQueryPool timeline_query_pool);

daxa_DeviceInfo const *
daxa_dvc_info(daxa_Device device);
VkDevice
daxa_dvc_get_vk_device(daxa_Device device);

void daxa_dvc_submit(daxa_Device device, daxa_CommandSubmitInfo const * info);
void daxa_dvc_present(daxa_Device device, daxa_PresentInfo const * info);
void daxa_dvc_wait_idle(daxa_Device device);
void daxa_dvc_collect_garbage(daxa_Device device);

#endif // #ifndef __DAXA_DEVICE_H__
