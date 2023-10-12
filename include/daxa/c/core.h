#ifndef __DAXA_CORE_H__
#define __DAXA_CORE_H__

#include <stdint.h>

#if defined(__cplusplus)
#define DAXA_EXPORT extern "C"
#else
#define DAXA_EXPORT
#endif

#define DAXA_SHADERLANG_GLSL 1
#define DAXA_SHADERLANG_HLSL 2

// TODO(capi): check all flags for abi compat.

typedef struct daxa_ImplDevice * daxa_Device;
typedef struct daxa_ImplCommandList * daxa_CommandList;
typedef struct daxa_ImplInstance * daxa_Instance;
typedef struct daxa_ImplComputePipeline * daxa_ComputePipeline;
typedef struct daxa_ImplRasterPipeline * daxa_RasterPipeline;
typedef struct daxa_ImplSwapchain * daxa_Swapchain;
typedef struct daxa_ImplBinarySemaphore * daxa_BinarySemaphore;
typedef struct daxa_ImplTimelineSemaphore * daxa_TimelineSemaphore;
typedef struct daxa_ImplEvent * daxa_Event;
typedef struct daxa_ImplTimelineQueryPool * daxa_TimelineQueryPool;
typedef struct daxa_ImplMemoryBlock * daxa_MemoryBlock;

typedef uint64_t daxa_Flags;

typedef char daxa_Bool8;

typedef struct
{
    char const * data;
    size_t size;
} daxa_StringView;

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

#define daxa_i32 int32_t
#define daxa_u32 uint32_t
#define daxa_i64 int64_t
#define daxa_u64 uint64_t
#define daxa_f32 float
#define daxa_f64 double

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201)
#endif

_DAXA_DECL_VEC2_TYPE(daxa_f32)
daxa_f32vec2;
_DAXA_DECL_VEC2_TYPE(daxa_f64)
daxa_f64vec2;
_DAXA_DECL_VEC2_TYPE(daxa_u32)
daxa_u32vec2;
_DAXA_DECL_VEC2_TYPE(daxa_i32)
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

_DAXA_DECL_VEC3_TYPE(daxa_f32)
daxa_f32vec3;
_DAXA_DECL_VEC3_TYPE(daxa_f64)
daxa_f64vec3;
_DAXA_DECL_VEC3_TYPE(daxa_u32)
daxa_u32vec3;
_DAXA_DECL_VEC3_TYPE(daxa_i32)
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
_DAXA_DECL_VEC4_TYPE(daxa_f32)
daxa_f32vec4;
_DAXA_DECL_VEC4_TYPE(daxa_f64)
daxa_f64vec4;
_DAXA_DECL_VEC4_TYPE(daxa_u32)
daxa_u32vec4;
_DAXA_DECL_VEC4_TYPE(daxa_i32)
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

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
