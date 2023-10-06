#ifndef __DAXA_GPU_RESOURCES_H__
#define __DAXA_GPU_RESOURCES_H__

#include <vk_mem_alloc.h>

#include <daxa/c/types.h>

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

DAXA_EXPORT daxa_ImageViewId
daxa_default_view(daxa_ImageId image);

DAXA_EXPORT uint32_t
daxa_index_of(uint32_t value);

DAXA_EXPORT uint32_t
daxa_version_of(uint32_t value);

typedef struct
{
    uint64_t address;
} daxa_BufferDeviceAddress;

typedef struct
{
    size_t size;
    daxa_Variant(daxa_AllocInfoUnion) allocate_info;
    daxa_StringView name;
} daxa_BufferInfo;

typedef uint32_t daxa_ImageFlags;
static const daxa_ImageFlags DAXA_IMAGE_FLAG_NONE = 0x00000000;
static const daxa_ImageFlags DAXA_IMAGE_FLAG_ALLOW_MUTABLE_FORMAT = 0x00000008;
static const daxa_ImageFlags DAXA_IMAGE_FLAG_COMPATIBLE_CUBE = 0x00000010;
static const daxa_ImageFlags DAXA_IMAGE_FLAG_COMPATIBLE_2D_ARRAY = 0x00000020;
static const daxa_ImageFlags DAXA_IMAGE_FLAG_ALLOW_ALIAS = 0x00000400;

typedef uint32_t daxa_ImageUsageFlags;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_NONE = 0x00000000;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_TRANSFER_SRC = 0x00000001;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_TRANSFER_DST = 0x00000002;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_SHADER_SAMPLED = 0x00000004;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_SHADER_STORAGE = 0x00000008;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_COLOR_ATTACHMENT = 0x00000010;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_DEPTH_STENCIL_ATTACHMENT = 0x00000020;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_TRANSIENT_ATTACHMENT = 0x00000040;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_FRAGMENT_DENSITY_MAP = 0x00000200;
static const daxa_ImageUsageFlags DAXA_IMAGE_USE_FLAG_FRAGMENT_SHADING_RATE_ATTACHMENT = 0x00000100;

typedef struct
{
    daxa_ImageFlags flags;
    uint32_t dimensions;
    VkFormat format;
    VkExtent3D size;
    uint32_t mip_level_count;
    uint32_t array_layer_count;
    uint32_t sample_count;
    daxa_ImageUsageFlags usage;
    daxa_Variant(daxa_AllocInfoUnion) allocate_info;
    daxa_StringView name;
} daxa_ImageInfo;

typedef struct
{
    VkImageViewType type;
    VkFormat format;
    daxa_ImageId image;
    daxa_ImageMipArraySlice slice;
    daxa_StringView name;
} daxa_ImageViewInfo;

typedef struct
{
    VkFilter magnification_filter;
    VkFilter minification_filter;
    VkFilter mipmap_filter;
    VkSamplerReductionMode reduction_mode;
    VkSamplerAddressMode address_mode_u;
    VkSamplerAddressMode address_mode_v;
    VkSamplerAddressMode address_mode_w;
    float mip_lod_bias;
    VkBool32 enable_anisotropy;
    float max_anisotropy;
    VkBool32 enable_compare;
    VkCompareOp compare_op;
    float min_lod;
    float max_lod;
    VkBorderColor border_color;
    VkBool32 enable_unnormalized_coordinates;
    daxa_StringView name;
} daxa_SamplerInfo;

static const daxa_BufferInfo DAXA_DEFAULT_BUFFER_INFO = {
    .size = 0,
    .allocate_info = {.index = 0, .values = {.auto_alloc_info = 0}},
    .name = 0,
};
static const daxa_ImageInfo DAXA_DEFAULT_IMAGE_INFO = {
    .flags = 0,
    .dimensions = 2,
    .format = VK_FORMAT_R8G8B8A8_SRGB,
    .size = {0, 0, 0},
    .mip_level_count = 1,
    .array_layer_count = 1,
    .sample_count = 1,
    .usage = 0,
    .allocate_info = {.index = 0, .values = {.auto_alloc_info = 0}},
    .name = 0,
};
static const daxa_ImageViewInfo DAXA_DEFAULT_IMAGE_VIEW_INFO = {
    .type = VK_IMAGE_VIEW_TYPE_2D,
    .format = VK_FORMAT_R8G8B8A8_SRGB,
    .name = 0,
};
static const daxa_SamplerInfo DAXA_DEFAULT_SAMPLER_INFO = {
    .magnification_filter = VK_FILTER_LINEAR,
    .minification_filter = VK_FILTER_LINEAR,
    .mipmap_filter = VK_FILTER_LINEAR,
    .reduction_mode = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE,
    .address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    .mip_lod_bias = 0.5f,
    .enable_anisotropy = 0,
    .max_anisotropy = 0.0f,
    .enable_compare = 0,
    .compare_op = VK_COMPARE_OP_ALWAYS,
    .min_lod = 0.0f,
    .max_lod = 1000.0f,
    .border_color = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
    .enable_unnormalized_coordinates = 0,
    .name = 0,
};

DAXA_EXPORT VmaAllocation
daxa_memory_block_get_vma_allocation(daxa_MemoryBlock memory_block);

#endif // #ifndef __DAXA_GPU_RESOURCES_H__
