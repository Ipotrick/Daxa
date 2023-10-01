#ifndef __DAXA_TYPES_H__
#define __DAXA_TYPES_H__

#include <vulkan/vulkan.h>
#include <stdint.h>

#if defined(__cplusplus)
#define DAXA_EXPORT extern "C"
#else
#define DAXA_EXPORT
#endif

typedef uint64_t daxa_Flags;

typedef char daxa_Bool8;

typedef struct
{
    char const * data;
    size_t size;
} daxa_StringView;

typedef enum
{
    DAXA_RESULT_UNKNOWN = 0,
    DAXA_RESULT_SUCCESS = 1,
    DAXA_RESULT_MISSING_EXTENSION = 2,
    DAXA_RESULT_INVALID_BUFFER_ID = (1 << 30) + 1,
    DAXA_RESULT_INVALID_IMAGE_ID = (1 << 30) + 2,
    DAXA_RESULT_INVALID_IMAGE_VIEW_ID =(1 << 30) + 3,
    DAXA_RESULT_INVALID_SAMPLER_ID = (1 << 30) + 4,
    DAXA_RESULT_BUFFER_DOUBLE_FREE = (1 << 30) + 5,
    DAXA_RESULT_IMAGE_DOUBLE_FREE = (1 << 30) + 6,
    DAXA_RESULT_IMAGE_VIEW_DOUBLE_FREE =(1 << 30) + 7,
    DAXA_RESULT_SAMPLER_DOUBLE_FREE = (1 << 30) + 8,
    DAXA_RESULT_INVALID_BUFFER_INFO = (1 << 30) + 9,
    DAXA_RESULT_INVALID_IMAGE_INFO = (1 << 30) + 10,
    DAXA_RESULT_INVALID_IMAGE_VIEW_INFO =(1 << 30) + 11,
    DAXA_RESULT_INVALID_SAMPLER_INFO = (1 << 30) + 12,
    DAXA_RESULT_MAX_ENUM = 0x7FFFFFFF,
} daxa_Result;

// ImageLayout matches vulkan's image layouts
typedef enum
{
    DAXA_IMAGE_LAYOUT_UNDEFINED = 0,
    DAXA_IMAGE_LAYOUT_GENERAL = 1,
    DAXA_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
    DAXA_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
    DAXA_IMAGE_LAYOUT_READ_ONLY_OPTIMAL = 1000314000,
    DAXA_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL = 1000314001,
    DAXA_IMAGE_LAYOUT_PRESENT_SRC = 1000001002,
    DAXA_IMAGE_LAYOUT_MAX_ENUM = 0x7FFFFFFF,
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

typedef uint32_t daxa_MemoryFlags;
static const daxa_MemoryFlags DAXA_MEMORY_FLAG_NONE = 0x00000000;
static const daxa_MemoryFlags DAXA_MEMORY_FLAG_DEDICATED_MEMORY = 0x00000001;
static const daxa_MemoryFlags DAXA_MEMORY_FLAG_CAN_ALIAS = 0x00000200;
static const daxa_MemoryFlags DAXA_MEMORY_FLAG_HOST_ACCESS_SEQUENTIAL_WRITE = 0x00000400;
static const daxa_MemoryFlags DAXA_MEMORY_FLAG_HOST_ACCESS_RANDOM = 0x00000800;
static const daxa_MemoryFlags DAXA_MEMORY_FLAG_STRATEGY_MIN_MEMORY = 0x00010000;
static const daxa_MemoryFlags DAXA_MEMORY_FLAG_STRATEGY_MIN_TIME = 0x00020000;

typedef struct
{
    VkMemoryRequirements requirements;
    daxa_MemoryFlags flags;
} daxa_MemoryBlockInfo;

struct daxa_ImplMemoryBlock;
typedef struct daxa_ImplMemoryBlock * daxa_MemoryBlock;

DAXA_EXPORT daxa_Result
daxa_memory_destroy(daxa_MemoryBlock memory_block);

typedef struct
{
    daxa_MemoryBlock memory_block;
    size_t offset;
} daxa_ManualAllocInfo;

typedef struct
{
    uint64_t index;
    union
    {
        daxa_MemoryFlags auto_alloc_info;
        daxa_ManualAllocInfo manual_alloc_info;
    };
} daxa_AllocateInfo;

typedef struct
{
    uint32_t query_count;
    daxa_StringView name;
} daxa_TimelineQueryPoolInfo;

typedef struct daxa_ImplTimelineQueryPool * daxa_TimelineQueryPool;

DAXA_EXPORT daxa_TimelineQueryPoolInfo const *
daxa_timeline_query_pool_info(daxa_TimelineQueryPool timeline_query_pool);

DAXA_EXPORT void
daxa_timeline_query_pool_query_results(daxa_TimelineQueryPool timeline_query_pool, uint64_t * out_counnt, uint64_t * out_results);

DAXA_EXPORT daxa_Result
daxa_destroy_timeline_query_pool(daxa_TimelineQueryPool timeline_query_pool);

#define _DAXA_DECL_OPTIONAL(T) \
    typedef struct             \
    {                          \
        T value;               \
        daxa_Bool8 has_value;  \
    } daxa_Optional##T;

#define daxa_Optional(T) daxa_Optional##T

#define _DAXA_DECL_FIXED_LIST(T, CAPACITY) \
    typedef struct                         \
    {                                      \
        T data[CAPACITY];                  \
        size_t size;                       \
    } daxa_FixedList##T##CAPACITY;

#define daxa_FixedList(T, CAPACITY) daxa_FixedList##T##CAPACITY

#endif // #ifndef __DAXA_TYPES_H__
