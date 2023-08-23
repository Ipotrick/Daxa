#pragma once

#if !defined(DAXA_STORAGE_BUFFER_BINDING)
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ID_INDEX_MASK (0x00FFFFFF)
#endif

typedef uint daxa_u32;
typedef bool daxa_b32;
typedef int daxa_i32;
typedef float daxa_f32;
typedef double daxa_f64;
typedef bool daxa_b32;
typedef bool2 daxa_b32vec2;
typedef bool3 daxa_b32vec3;
typedef bool4 daxa_b32vec4;
typedef float daxa_f32;
typedef double daxa_f64;
typedef float2 daxa_f32vec2;
typedef float2x2 daxa_f32mat2x2;
typedef float2x3 daxa_f32mat2x3;
typedef float2x4 daxa_f32mat2x4;
typedef double2 daxa_f64vec2;
typedef double2x2 daxa_f64mat2x2;
typedef double2x3 daxa_f64mat2x3;
typedef double2x4 daxa_f64mat2x4;
typedef float3 daxa_f32vec3;
typedef float3x2 daxa_f32mat3x2;
typedef float3x3 daxa_f32mat3x3;
typedef float3x4 daxa_f32mat3x4;
typedef double3 daxa_f64vec3;
typedef double3x2 daxa_f64mat3x2;
typedef double3x3 daxa_f64mat3x3;
typedef double3x4 daxa_f64mat3x4;
typedef float4 daxa_f32vec4;
typedef float4x2 daxa_f32mat4x2;
typedef float4x3 daxa_f32mat4x3;
typedef float4x4 daxa_f32mat4x4;
typedef double4 daxa_f64vec4;
typedef double4x2 daxa_f64mat4x2;
typedef double4x3 daxa_f64mat4x3;
typedef double4x4 daxa_f64mat4x4;
typedef float4 daxa_f32vec4;
typedef float4x2 daxa_f32mat4x2;
typedef float4x3 daxa_f32mat4x3;
typedef float4x4 daxa_f32mat4x4;
typedef int daxa_i32;
typedef int2 daxa_i32vec2;
typedef int2x2 daxa_i32mat2x2;
typedef int2x3 daxa_i32mat2x3;
typedef int2x4 daxa_i32mat2x4;
typedef uint2 daxa_u32vec2;
typedef uint2x2 daxa_u32mat2x2;
typedef uint2x3 daxa_u32mat2x3;
typedef uint2x4 daxa_u32mat2x4;
typedef int3 daxa_i32vec3;
typedef int3x2 daxa_i32mat3x2;
typedef int3x3 daxa_i32mat3x3;
typedef int3x4 daxa_i32mat3x4;
typedef uint3 daxa_u32vec3;
typedef uint3x2 daxa_u32mat3x2;
typedef uint3x3 daxa_u32mat3x3;
typedef uint3x4 daxa_u32mat3x4;
typedef int4 daxa_i32vec4;
typedef int4x2 daxa_i32mat4x2;
typedef int4x3 daxa_i32mat4x3;
typedef int4x4 daxa_i32mat4x4;
typedef uint4 daxa_u32vec4;
typedef uint4x2 daxa_u32mat4x2;
typedef uint4x3 daxa_u32mat4x3;
typedef uint4x4 daxa_u32mat4x4;

namespace daxa
{
    struct BufferId
    {
        daxa_u32 value;
        daxa_u32 index()
        {
            return (DAXA_ID_INDEX_MASK & value);
        }
        daxa_u32 version()
        {
            return (value >> 24);
        }
        template <typename T>
        T get();
    };

    struct ImageViewId
    {
        daxa_u32 value;
        daxa_u32 index()
        {
            return (DAXA_ID_INDEX_MASK & value);
        }
        daxa_u32 version()
        {
            return (value >> 24);
        }
        template <typename T>
        T get();
    };

    struct SamplerId
    {
        daxa_u32 value;
        daxa_u32 index()
        {
            return (DAXA_ID_INDEX_MASK & value);
        }
        daxa_u32 version()
        {
            return (value >> 24);
        }
        SamplerState get();
    };
} // namespace daxa

#define daxa_BufferId daxa::BufferId
#define daxa_ImageViewId daxa::ImageViewId
#define daxa_SamplerId daxa::SamplerId

#define _DAXA_DECL_GET_TEX_HEADER(TYPE) \
    template <typename T>               \
    TYPE<T> get_##TYPE(daxa::ImageViewId image_id);

namespace daxa
{
    template <typename T>
    StructuredBuffer<T> get_StructuredBuffer(daxa::BufferId buffer_id);

    _DAXA_DECL_GET_TEX_HEADER(Texture1D)
    _DAXA_DECL_GET_TEX_HEADER(Texture1DArray)
    _DAXA_DECL_GET_TEX_HEADER(Texture2D)
    _DAXA_DECL_GET_TEX_HEADER(Texture2DArray)
    _DAXA_DECL_GET_TEX_HEADER(Texture2DMS)
    _DAXA_DECL_GET_TEX_HEADER(Texture2DMSArray)
    _DAXA_DECL_GET_TEX_HEADER(Texture3D)
    _DAXA_DECL_GET_TEX_HEADER(TextureCube)
    _DAXA_DECL_GET_TEX_HEADER(TextureCubeArray)
    _DAXA_DECL_GET_TEX_HEADER(RWTexture1D)
    _DAXA_DECL_GET_TEX_HEADER(RWTexture1DArray)
    _DAXA_DECL_GET_TEX_HEADER(RWTexture2D)
    _DAXA_DECL_GET_TEX_HEADER(RWTexture2DArray)
    _DAXA_DECL_GET_TEX_HEADER(RWTexture3D)

    [[vk::binding(DAXA_SAMPLER_BINDING, 0)]] SamplerState SamplerStateTable[];
    SamplerState SamplerId::get()
    {
        return SamplerStateTable[index()];
    }

    [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] ByteAddressBuffer ByteAddressBufferTable[];
    template <>
    ByteAddressBuffer daxa::BufferId::get()
    {
        return ByteAddressBufferTable[index()];
    }

    [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] RWByteAddressBuffer RWByteAddressBufferTable[];
    template <>
    RWByteAddressBuffer daxa::BufferId::get()
    {
        return RWByteAddressBufferTable[index()];
    }
} // namespace daxa

#define DAXA_DECL_BUFFER_PTR_ALIGN(Type, Align) DAXA_DECL_BUFFER_PTR(Type)
#define DAXA_DECL_BUFFER_PTR(Type)                                                                              \
    namespace daxa                                                                                              \
    {                                                                                                           \
        [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] StructuredBuffer<Type> StructuredBuffer##Type##Table[]; \
        template <>                                                                                             \
        StructuredBuffer<Type> daxa::BufferId::get()                                                            \
        {                                                                                                       \
            return StructuredBuffer##Type##Table[index()];                                                      \
        }                                                                                                       \
    }

#define _DAXA_DECL_TEXTURE_RET(TYPE, RET_TYPE)                                             \
    [[vk::binding(DAXA_SAMPLED_IMAGE_BINDING, 0)]] TYPE<RET_TYPE> TYPE##RET_TYPE##Table[]; \
    template <>                                                                            \
    TYPE<RET_TYPE> daxa::ImageViewId::get()                                                \
    {                                                                                      \
        return TYPE##RET_TYPE##Table[index()];                                             \
    }

#define _DAXA_DECL_RWTEXTURE_RET(TYPE, RET_TYPE)                                                   \
    [[vk::binding(DAXA_STORAGE_IMAGE_BINDING, 0)]] RW##TYPE<RET_TYPE> RW##TYPE##RET_TYPE##Table[]; \
    template <>                                                                                    \
    RW##TYPE<RET_TYPE> daxa::ImageViewId::get()                                                    \
    {                                                                                              \
        return RW##TYPE##RET_TYPE##Table[index()];                                                 \
    }

#define _DAXA_DECL_TEXTURE(TYPE)           \
    _DAXA_DECL_TEXTURE_RET(TYPE, float4)   \
    _DAXA_DECL_TEXTURE_RET(TYPE, uint4)    \
    _DAXA_DECL_TEXTURE_RET(TYPE, int4)     \
    _DAXA_DECL_RWTEXTURE_RET(TYPE, float4) \
    _DAXA_DECL_RWTEXTURE_RET(TYPE, uint4)  \
    _DAXA_DECL_RWTEXTURE_RET(TYPE, int4)

#define _DAXA_DECL_TEXTURE_NO_RW(TYPE)   \
    _DAXA_DECL_TEXTURE_RET(TYPE, float4) \
    _DAXA_DECL_TEXTURE_RET(TYPE, uint4)  \
    _DAXA_DECL_TEXTURE_RET(TYPE, int4)

namespace daxa
{
    _DAXA_DECL_TEXTURE(Texture1D)
    _DAXA_DECL_TEXTURE(Texture1DArray)
    _DAXA_DECL_TEXTURE(Texture2D)
    _DAXA_DECL_TEXTURE(Texture2DArray)
    _DAXA_DECL_TEXTURE(Texture3D)
    _DAXA_DECL_TEXTURE_NO_RW(Texture2DMS)
    _DAXA_DECL_TEXTURE_NO_RW(Texture2DMSArray)
    _DAXA_DECL_TEXTURE_NO_RW(TextureCube)
    _DAXA_DECL_TEXTURE_NO_RW(TextureCubeArray)
} // namespace daxa

#if DAXA_ENABLE_SHADER_NO_NAMESPACE
#define b32vec1 daxa_b32vec1
#define b32vec2 daxa_b32vec2
#define b32vec3 daxa_b32vec3
#define b32vec4 daxa_b32vec4
#define f32 daxa_f32
#define f32vec1 daxa_f32vec1
#define f32vec2 daxa_f32vec2
#define f32mat2x2 daxa_f32mat2x2
#define f32mat2x3 daxa_f32mat2x3
#define f32mat2x4 daxa_f32mat2x4
#define f32vec3 daxa_f32vec3
#define f32mat3x2 daxa_f32mat3x2
#define f32mat3x3 daxa_f32mat3x3
#define f32mat3x4 daxa_f32mat3x4
#define f32vec4 daxa_f32vec4
#define f32mat4x2 daxa_f32mat4x2
#define f32mat4x3 daxa_f32mat4x3
#define f32mat4x4 daxa_f32mat4x4
#define i32 daxa_i32
#define i32vec1 daxa_i32vec1
#define i32vec2 daxa_i32vec2
#define i32vec3 daxa_i32vec3
#define i32vec4 daxa_i32vec4
#define u32 daxa_u32
#define u32vec1 daxa_u32vec1
#define u32vec2 daxa_u32vec2
#define u32vec3 daxa_u32vec3
#define u32vec4 daxa_u32vec4
#define i64 daxa_i64
#define i64vec1 daxa_i64vec1
#define u64 daxa_u64
#define u64vec1 daxa_u64
#endif
