#ifndef __DAXA_CORE_H__
#define __DAXA_CORE_H__

#include <stdint.h>

#if defined(__cplusplus)
#define DAXA_EXPORT extern "C"
#define DAXA_ZERO_INIT {}
#else
#define DAXA_EXPORT
#define DAXA_ZERO_INIT {0}
#endif

#if defined(__cplusplus)
#define DAXA_NO_DISCARD [[nodiscard]]
#else
#define DAXA_NO_DISCARD
#endif

#define _DAXA_TEST_PRINT(...)

#define DAXA_SHADERLANG_GLSL 1
#define DAXA_SHADERLANG_HLSL 2

static const uint32_t DAXA_ID_INDEX_BITS = 20;
static const uint32_t DAXA_ID_INDEX_MASK = (1ull << 20) - 1ull;
static const uint32_t DAXA_ID_INDEX_OFFSET = 0;
static const uint32_t DAXA_ID_VERSION_BITS = 44;
static const uint64_t DAXA_ID_VERSION_MASK = (1ull << 44) - 1ull;
static const uint32_t DAXA_ID_VERSION_OFFSET = 20;

typedef struct daxa_ImplDevice * daxa_Device;
typedef struct daxa_ImplCommandRecorder * daxa_CommandRecorder;
typedef struct daxa_ImplCommandRecorder * daxa_CommandRecorder;
typedef struct daxa_ImplExecutableCommandList * daxa_ExecutableCommandList;
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
    uint64_t value;
} daxa_BufferId;

typedef struct
{
    uint64_t value;
} daxa_ImageId;

typedef struct
{
    uint64_t value;
} daxa_ImageViewId;

typedef struct
{
    uint64_t value;
} daxa_SamplerId;

#define _DAXA_DECL_VEC2_TYPE(SCALAR_TYPE) \
    typedef struct                        \
    {                                     \
        SCALAR_TYPE x;                    \
        SCALAR_TYPE y;                    \
    }
typedef uint32_t daxa_b32;
typedef int32_t daxa_i32;
typedef uint32_t daxa_u32;
typedef int64_t daxa_i64;
typedef uint64_t daxa_u64;
typedef float daxa_f32;
typedef double daxa_f64;

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
    typedef struct                        \
    {                                     \
        SCALAR_TYPE x;                    \
        SCALAR_TYPE y;                    \
        SCALAR_TYPE z;                    \
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
    typedef struct                        \
    {                                     \
        SCALAR_TYPE x;                    \
        SCALAR_TYPE y;                    \
        SCALAR_TYPE z;                    \
        SCALAR_TYPE w;                    \
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
