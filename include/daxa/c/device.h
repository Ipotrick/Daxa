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

typedef struct
{
    uint32_t max_image_dimension1d;
    uint32_t max_image_dimension2d;
    uint32_t max_image_dimension3d;
    uint32_t max_image_dimension_cube;
    uint32_t max_image_array_layers;
    uint32_t max_texel_buffer_elements;
    uint32_t max_uniform_buffer_range;
    uint32_t max_storage_buffer_range;
    uint32_t max_push_constants_size;
    uint32_t max_memory_allocation_count;
    uint32_t max_sampler_allocation_count;
    uint64_t buffer_image_granularity;
    uint64_t sparse_address_space_size;
    uint32_t max_bound_descriptor_sets;
    uint32_t max_per_stage_descriptor_samplers;
    uint32_t max_per_stage_descriptor_uniform_buffers;
    uint32_t max_per_stage_descriptor_storage_buffers;
    uint32_t max_per_stage_descriptor_sampled_images;
    uint32_t max_per_stage_descriptor_storage_images;
    uint32_t max_per_stage_descriptor_input_attachments;
    uint32_t max_per_stage_resources;
    uint32_t max_descriptor_set_samplers;
    uint32_t max_descriptor_set_uniform_buffers;
    uint32_t max_descriptor_set_uniform_buffers_dynamic;
    uint32_t max_descriptor_set_storage_buffers;
    uint32_t max_descriptor_set_storage_buffers_dynamic;
    uint32_t max_descriptor_set_sampled_images;
    uint32_t max_descriptor_set_storage_images;
    uint32_t max_descriptor_set_input_attachments;
    uint32_t max_vertex_input_attributes;
    uint32_t max_vertex_input_bindings;
    uint32_t max_vertex_input_attribute_offset;
    uint32_t max_vertex_input_binding_stride;
    uint32_t max_vertex_output_components;
    uint32_t max_tessellation_generation_level;
    uint32_t max_tessellation_patch_size;
    uint32_t max_tessellation_control_per_vertex_input_components;
    uint32_t max_tessellation_control_per_vertex_output_components;
    uint32_t max_tessellation_control_per_patch_output_components;
    uint32_t max_tessellation_control_total_output_components;
    uint32_t max_tessellation_evaluation_input_components;
    uint32_t max_tessellation_evaluation_output_components;
    uint32_t max_geometry_shader_invocations;
    uint32_t max_geometry_input_components;
    uint32_t max_geometry_output_components;
    uint32_t max_geometry_output_vertices;
    uint32_t max_geometry_total_output_components;
    uint32_t max_fragment_input_components;
    uint32_t max_fragment_output_attachments;
    uint32_t max_fragment_dual_src_attachments;
    uint32_t max_fragment_combined_output_resources;
    uint32_t max_compute_shared_memory_size;
    uint32_t max_compute_work_group_count[3];
    uint32_t max_compute_work_group_invocations;
    uint32_t max_compute_work_group_size[3];
    uint32_t sub_pixel_precision_bits;
    uint32_t sub_texel_precision_bits;
    uint32_t mipmap_precision_bits;
    uint32_t max_draw_indexed_index_value;
    uint32_t max_draw_indirect_count;
    float max_sampler_lod_bias;
    float max_sampler_anisotropy;
    uint32_t max_viewports;
    uint32_t max_viewport_dimensions[2];
    float viewport_bounds_range[2];
    uint32_t viewport_sub_pixel_bits;
    size_t min_memory_map_alignment;
    uint64_t min_texel_buffer_offset_alignment;
    uint64_t min_uniform_buffer_offset_alignment;
    uint64_t min_storage_buffer_offset_alignment;
    int32_t min_texel_offset;
    uint32_t max_texel_offset;
    int32_t min_texel_gather_offset;
    uint32_t max_texel_gather_offset;
    float min_interpolation_offset;
    float max_interpolation_offset;
    uint32_t sub_pixel_interpolation_offset_bits;
    uint32_t max_framebuffer_width;
    uint32_t max_framebuffer_height;
    uint32_t max_framebuffer_layers;
    uint32_t framebuffer_color_sample_counts;
    uint32_t framebuffer_depth_sample_counts;
    uint32_t framebuffer_stencil_sample_counts;
    uint32_t framebuffer_no_attachments_sample_counts;
    uint32_t max_color_attachments;
    uint32_t sampled_image_color_sample_counts;
    uint32_t sampled_image_integer_sample_counts;
    uint32_t sampled_image_depth_sample_counts;
    uint32_t sampled_image_stencil_sample_counts;
    uint32_t storage_image_sample_counts;
    uint32_t max_sample_mask_words;
    uint32_t timestamp_compute_and_graphics;
    float timestamp_period;
    uint32_t max_clip_distances;
    uint32_t max_cull_distances;
    uint32_t max_combined_clip_and_cull_distances;
    uint32_t discrete_queue_priorities;
    float point_size_range[2];
    float line_width_range[2];
    float point_size_granularity;
    float line_width_granularity;
    uint32_t strict_lines;
    uint32_t standard_sample_locations;
    uint64_t optimal_buffer_copy_offset_alignment;
    uint64_t optimal_buffer_copy_row_pitch_alignment;
    uint64_t non_coherent_atom_size;
} daxa_DeviceLimits;

typedef struct
{
    uint32_t vulkan_api_version;
    uint32_t driver_version;
    uint32_t vendor_id;
    uint32_t device_id;
    daxa_DeviceType device_type;
    char device_name[256U];
    char pipeline_cache_uuid[16U];
    daxa_DeviceLimits limits;
} daxa_DeviceProperties;

DAXA_EXPORT int32_t
daxa_default_device_score(daxa_DeviceProperties const * properties);

typedef enum
{
    DAXA_DEVICE_FLAG_BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT = 0x1 << 0,
    DAXA_DEVICE_FLAG_CONSERVATIVE_RASTERIZATION = 0x1 << 1,
    DAXA_DEVICE_FLAG_MESH_SHADER_BIT = 0x1 << 2,
    DAXA_DEVICE_FLAG_SHADER_ATOMIC64 = 0x1 << 3,
    DAXA_DEVICE_FLAG_IMAGE_ATOMIC64 = 0x1 << 4,
    DAXA_DEVICE_FLAG_VK_MEMORY_MODEL = 0x1 << 5,
} daxa_DeviceFlagBits;

typedef uint32_t daxa_DeviceFlags;

typedef struct
{
    int32_t (*selector)(daxa_DeviceProperties const * properties);
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
    .max_allowed_samplers = 400,
    .name = {.data = {}, .size = 0},
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

typedef struct
{
    daxa_BufferInfo buffer_info;
    daxa_MemoryBlock * memory_block;
    size_t offset;
} daxa_MemoryBlockBufferInfo;

static const daxa_MemoryBlockBufferInfo DAXA_DEFAULT_MEMORY_BLOCK_BUFFER_INFO = {};

typedef struct
{
    daxa_ImageInfo image_info;
    daxa_MemoryBlock * memory_block;
    size_t offset;
} daxa_MemoryBlockImageInfo;

static const daxa_MemoryBlockImageInfo DAXA_DEFAULT_MEMORY_BLOCK_IMAGE_INFO = {};

static const daxa_PresentInfo DAXA_DEFAULT_PRESENT_INFO = {};

DAXA_EXPORT VkMemoryRequirements
daxa_dvc_buffer_memory_requirements(daxa_Device device, daxa_BufferInfo const * info);
DAXA_EXPORT VkMemoryRequirements
daxa_dvc_image_memory_requirements(daxa_Device device, daxa_ImageInfo const * info);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_memory(daxa_Device device, daxa_MemoryBlockInfo const * info, daxa_MemoryBlock * out_memory_block);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_buffer(daxa_Device device, daxa_BufferInfo const * info, daxa_BufferId * out_id);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_image(daxa_Device device, daxa_ImageInfo const * info, daxa_ImageId * out_id);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_image_view(daxa_Device device, daxa_ImageViewInfo const * info, daxa_ImageViewId * out_id);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_sampler(daxa_Device device, daxa_SamplerInfo const * info, daxa_SamplerId * out_id);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_buffer_from_block(daxa_Device device, daxa_MemoryBlockBufferInfo const * info, daxa_BufferId * out_id);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_image_from_block(daxa_Device device, daxa_MemoryBlockImageInfo const * info, daxa_ImageId * out_id);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_destroy_buffer(daxa_Device device, daxa_BufferId buffer);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_destroy_image(daxa_Device device, daxa_ImageId image);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_destroy_image_view(daxa_Device device, daxa_ImageViewId id);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_destroy_sampler(daxa_Device device, daxa_SamplerId sampler);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_info_buffer(daxa_Device device, daxa_BufferId buffer, daxa_BufferInfo * out_info);
DAXA_EXPORT daxa_Result
daxa_dvc_info_image(daxa_Device device, daxa_ImageId image, daxa_ImageInfo * out_info);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_info_image_view(daxa_Device device, daxa_ImageViewId id, daxa_ImageViewInfo * out_info);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_info_sampler(daxa_Device device, daxa_SamplerId sampler, daxa_SamplerInfo * out_info);

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

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_buffer_device_address(daxa_Device device, daxa_BufferId buffer, daxa_BufferDeviceAddress * out_bda);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_buffer_host_address(daxa_Device device, daxa_BufferId buffer, void ** out_ptr);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_raster_pipeline(daxa_Device device, daxa_RasterPipelineInfo const * info, daxa_RasterPipeline * out_pipeline);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_compute_pipeline(daxa_Device device, daxa_ComputePipelineInfo const * info, daxa_ComputePipeline * out_pipeline);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_swapchain(daxa_Device device, daxa_SwapchainInfo const * info, daxa_Swapchain * out_swapchain);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_command_list(daxa_Device device, daxa_CommandListInfo const * info, daxa_CommandList * out_command_list);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_binary_semaphore(daxa_Device device, daxa_BinarySemaphoreInfo const * info, daxa_BinarySemaphore * out_binary_semaphore);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_timeline_semaphore(daxa_Device device, daxa_TimelineSemaphoreInfo const * info, daxa_TimelineSemaphore * out_timeline_semaphore);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_event(daxa_Device device, daxa_EventInfo const * info, daxa_Event * out_event);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_create_timeline_query_pool(daxa_Device device, daxa_TimelineQueryPoolInfo const * info, daxa_TimelineQueryPool * out_timeline_query_pool);

DAXA_EXPORT daxa_DeviceInfo const *
daxa_dvc_info(daxa_Device device);
DAXA_EXPORT VkDevice
daxa_dvc_get_vk_device(daxa_Device device);

DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_wait_idle(daxa_Device device);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_submit(daxa_Device device, daxa_CommandSubmitInfo const * info);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_present(daxa_Device device, daxa_PresentInfo const * info);
DAXA_EXPORT DAXA_NO_DISCARD daxa_Result
daxa_dvc_collect_garbage(daxa_Device device);
DAXA_EXPORT daxa_DeviceProperties const *
daxa_dvc_properties(daxa_Device device);

// Returns previous ref count.
DAXA_EXPORT uint64_t
daxa_dvc_inc_refcnt(daxa_Device device);
// Returns previous ref count.
DAXA_EXPORT uint64_t
daxa_dvc_dec_refcnt(daxa_Device device);

#endif // #ifndef __DAXA_DEVICE_H__
