#ifndef __DAXA_TYPES_H__
#define __DAXA_TYPES_H__

#include <vulkan.h>
#include <stdint.h>

typedef enum
{
    DAXA_RESULT_UNKNOWN = 0,
    DAXA_RESULT_OUT_OF_DEVICE_MEMORY = 1,
    DAXA_RESULT_MAX_ENUM = 0xFFFFFFFF,
} daxa_Result;

// ImageLayout matchs vulkans image layouts
typedef enum
{
    DAXA_IMAGE_LAYOUT_UNDEFINED = 0,
    DAXA_IMAGE_LAYOUT_GENERAL = 1,
    DAXA_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
    DAXA_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
    DAXA_IMAGE_LAYOUT_READ_ONLY_OPTIMAL = 1000314000,
    DAXA_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL = 1000314001,
    DAXA_IMAGE_LAYOUT_PRESENT_SRC = 1000001002,
    DAXA_IMAGE_LAYOUT_MAX_ENUM = 0x7fffffff,
} daxa_ImageLayout;

#define _DAXA_DECL_VEC2_TYPE(SCALAR_TYPE) \
    typedef union                         \
    {                                     \
        struct                            \
        {                                 \
            SCALAR_TYPE x;                \
            SCALAR_TYPE y;                \
        };                                \
        SCALAR_TYPE data[2];              \
    }

_DAXA_DECL_VEC2_TYPE(float)
daxa_f32vec2;
_DAXA_DECL_VEC2_TYPE(double)
daxa_f64vec2;
_DAXA_DECL_VEC2_TYPE(uint32_t)
daxa_u32vec2;
_DAXA_DECL_VEC2_TYPE(int32_t)
daxa_i32vec2;

#define _DAXA_DECL_VEC3_TYPE(SCALAR_TYPE) \
    typedef union                         \
    {                                     \
        struct                            \
        {                                 \
            SCALAR_TYPE x;                \
            SCALAR_TYPE y;                \
            SCALAR_TYPE z;                \
        };                                \
        SCALAR_TYPE data[3];              \
    }

_DAXA_DECL_VEC3_TYPE(float)
daxa_f32vec3;
_DAXA_DECL_VEC3_TYPE(double)
daxa_f64vec3;
_DAXA_DECL_VEC3_TYPE(uint32_t)
daxa_u32vec3;
_DAXA_DECL_VEC3_TYPE(int32_t)
daxa_i32vec3;

#define _DAXA_DECL_VEC4_TYPE(SCALAR_TYPE) \
    typedef union                         \
    {                                     \
        struct                            \
        {                                 \
            SCALAR_TYPE x;                \
            SCALAR_TYPE y;                \
            SCALAR_TYPE z;                \
            SCALAR_TYPE w;                \
        };                                \
        SCALAR_TYPE data[4];              \
    }

_DAXA_DECL_VEC4_TYPE(float)
daxa_f32vec4;
_DAXA_DECL_VEC4_TYPE(double)
daxa_f64vec4;
_DAXA_DECL_VEC4_TYPE(uint32_t)
daxa_u32vec4;
_DAXA_DECL_VEC4_TYPE(int32_t)
daxa_i32vec4;
_DAXA_DECL_VEC2_TYPE(daxa_f32vec2)
daxa_f32mat2x2;
_DAXA_DECL_VEC3_TYPE(daxa_f32vec2)
daxa_f32mat2x3;
_DAXA_DECL_VEC4_TYPE(daxa_f32vec2)
daxa_f32mat2x4;
_DAXA_DECL_VEC2_TYPE(daxa_f64vec2)
daxa_f64mat2x2;
_DAXA_DECL_VEC3_TYPE(daxa_f64vec2)
daxa_f64mat2x3;
_DAXA_DECL_VEC4_TYPE(daxa_f64vec2)
daxa_f64mat2x4;
_DAXA_DECL_VEC2_TYPE(daxa_f32vec3)
daxa_f32mat3x2;
_DAXA_DECL_VEC3_TYPE(daxa_f32vec3)
daxa_f32mat3x3;
_DAXA_DECL_VEC4_TYPE(daxa_f32vec3)
daxa_f32mat3x4;
_DAXA_DECL_VEC2_TYPE(daxa_f64vec3)
daxa_f64mat3x2;
_DAXA_DECL_VEC3_TYPE(daxa_f64vec3)
daxa_f64mat3x3;
_DAXA_DECL_VEC4_TYPE(daxa_f64vec3)
daxa_f64mat3x4;
_DAXA_DECL_VEC2_TYPE(daxa_f32vec4)
daxa_f32mat4x2;
_DAXA_DECL_VEC3_TYPE(daxa_f32vec4)
daxa_f32mat4x3;
_DAXA_DECL_VEC4_TYPE(daxa_f32vec4)
daxa_f32mat4x4;
_DAXA_DECL_VEC2_TYPE(daxa_f64vec4)
daxa_f64mat4x2;
_DAXA_DECL_VEC3_TYPE(daxa_f64vec4)
daxa_f64mat4x3;
_DAXA_DECL_VEC4_TYPE(daxa_f64vec4)
daxa_f64mat4x4;
_DAXA_DECL_VEC2_TYPE(daxa_i32vec2)
daxa_i32mat2x2;
_DAXA_DECL_VEC3_TYPE(daxa_i32vec2)
daxa_i32mat2x3;
_DAXA_DECL_VEC4_TYPE(daxa_i32vec2)
daxa_i32mat2x4;
_DAXA_DECL_VEC2_TYPE(daxa_u32vec2)
daxa_u32mat2x2;
_DAXA_DECL_VEC3_TYPE(daxa_u32vec2)
daxa_u32mat2x3;
_DAXA_DECL_VEC4_TYPE(daxa_u32vec2)
daxa_u32mat2x4;
_DAXA_DECL_VEC2_TYPE(daxa_i32vec3)
daxa_i32mat3x2;
_DAXA_DECL_VEC3_TYPE(daxa_i32vec3)
daxa_i32mat3x3;
_DAXA_DECL_VEC4_TYPE(daxa_i32vec3)
daxa_i32mat3x4;
_DAXA_DECL_VEC2_TYPE(daxa_u32vec3)
daxa_u32mat3x2;
_DAXA_DECL_VEC3_TYPE(daxa_u32vec3)
daxa_u32mat3x3;
_DAXA_DECL_VEC4_TYPE(daxa_u32vec3)
daxa_u32mat3x4;
_DAXA_DECL_VEC2_TYPE(daxa_i32vec4)
daxa_i32mat4x2;
_DAXA_DECL_VEC3_TYPE(daxa_i32vec4)
daxa_i32mat4x3;
_DAXA_DECL_VEC4_TYPE(daxa_i32vec4)
daxa_i32mat4x4;
_DAXA_DECL_VEC2_TYPE(daxa_u32vec4)
daxa_u32mat4x2;
_DAXA_DECL_VEC3_TYPE(daxa_u32vec4)
daxa_u32mat4x3;
_DAXA_DECL_VEC4_TYPE(daxa_u32vec4)
daxa_u32mat4x4;

typedef struct
{
    uint32_t base_mip_level;
    uint32_t level_count;
    uint32_t base_array_layer;
    uint32_t layer_count;
} daxa_ImageMipArraySlice;

typedef struct
{
    uint32_t mip_level;
    uint32_t base_array_layer;
    uint32_t layer_count;
} daxa_ImageArraySlice;

typedef struct
{
    uint32_t mip_level;
    uint32_t array_layer;
} daxa_ImageSlice;

typedef enum
{
    DAXA_MEMORY_FLAG_NONE = 0x00000000,
    DAXA_MEMORY_FLAG_DEDICATED_MEMORY = 0x00000001,
    DAXA_MEMORY_FLAG_CAN_ALIAS = 0x00000200,
    DAXA_MEMORY_FLAG_HOST_ACCESS_SEQUENTIAL_WRITE = 0x00000400,
    DAXA_MEMORY_FLAG_HOST_ACCESS_RANDOM = 0x00000800,
    DAXA_MEMORY_FLAG_STRATEGY_MIN_MEMORY = 0x00010000,
    DAXA_MEMORY_FLAG_STRATEGY_MIN_TIME = 0x00020000,
} daxa_MemoryFlagBits;

typedef uint64_t daxa_MemoryFlags;

typedef struct 
{
    VkMemoryRequirements requirements;
    daxa_MemoryFlagBits flags;
} daxa_MemoryBlockInfo;

struct daxa_ImplMemoryBlock;
typedef struct daxa_ImplMemoryBlock * daxa_MemoryBlock;

struct ManualAllocInfo
{
    daxa_MemoryBlock memory_block;
    size_t offset;
};

typedef struct
{
    uint64_t index;
    union
    {
        daxa_MemoryFlags auto_alloc_info;
        daxa_ManualAllocInfo manual_alloc_info;
    };
} daxa_AllocateInfo;

#endif // #ifndef __DAXA_TYPES_H__