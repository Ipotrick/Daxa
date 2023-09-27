#ifndef __DAXA_GPU_RESOURCES_H__
#define __DAXA_GPU_RESOURCES_H__

#include "types.h"

typedef struct
{
    uint32_t value;
} daxa_BufferId;

typedef struct
{
    uint32_t value;
} daxa_ImageId;

typedef struct
{
    uint32_t value;
} daxa_ImageViewId;

typedef struct
{
    uint32_t value;
} daxa_SamplerId;

daxa_ImageViewId
daxa_default_view(daxa_ImageId image);

uint32_t
daxa_index_of(uint32_t value);

uint32_t
daxa_version_of(uint32_t value);

typedef struct
{
    uint64_t address;
} daxa_BufferDeviceAddress;

typedef struct
{
    size_t size;
    daxa_AllocateInfo allocate_info;
    char const * name;
} daxa_BufferInfo;

typedef struct
{
    daxa_ImageCreateFlags flags;
    uint32_t dimensions;
    VkFormat format;
    VkExtent3D size;
    uint32_t mip_level_count;
    uint32_t array_layer_count;
    uint32_t sample_count;
    daxa_ImageUsageFlags usage;
    daxa_AllocateInfo allocate_info;
    char const * name;
} daxa_ImageInfo;

typedef struct
{
    VkImageViewType type;
    VkFormat format;
    daxa_ImageId image;
    daxa_ImageMipArraySlice slice;
    char const * name;
} daxa_ImageViewInfo;

typedef struct 
{
    VkFilter magnification_filter;
    VkFilter minification_filter;
    VkFilter mipmap_filter;
    VkReductionMode reduction_mode;
    VkSamplerAddressMode address_mode_u;
    VkSamplerAddressMode address_mode_v;
    VkSamplerAddressMode address_mode_w;
    float mip_lod_bias;
    VkBool enable_anisotropy;
    float max_anisotropy;
    VkBool enable_compare;
    CompareOp compare_op;
    float min_lod;
    float max_lod;
    VkBorderColor border_color;
    VkBool enable_unnormalized_coordinates;
    char const * name;
} daxa_SamplerInfo;

extern daxa_BufferInfo const * DAXA_DEFAULT_BUFFER_INFO;
extern daxa_ImageInfo const * DAXA_DEFAULT_IMAGE_INFO;
extern daxa_ImageViewInfo const * DAXA_DEFAULT_IMAGE_VIEW_INFO;
extern daxa_SamplerInfo const * DAXA_DEFAULT_SAMPLER_INFO;

VkMemory
daxa_memory_block_get_vk_memory(daxa_MemoryBlock memory_block);

#endif // #ifndef __DAXA_GPU_RESOURCES_H__