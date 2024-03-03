#pragma once

#if !defined(DAXA_GPU_TABLE_SET_BINDING)
#define DAXA_GPU_TABLE_SET_BINDING 0
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ACCELERATION_STRUCTURE_BINDING 5
#endif

#define DAXA_ID_INDEX_BITS 20
#define DAXA_ID_INDEX_MASK ((uint64_t(1) << DAXA_ID_INDEX_BITS) - uint64_t(1))
#define DAXA_ID_INDEX_OFFSTET 0
#define DAXA_ID_VERSION_BITS 44
#define DAXA_ID_VERSION_MASK ((uint64_t(1) << DAXA_ID_VERSION_BITS) - uint64_t(1))
#define DAXA_ID_VERSION_OFFSTET DAXA_ID_INDEX_BITS

typedef uint daxa_u32;
typedef int daxa_i32;
typedef uint64_t daxa_u64;
typedef int64_t daxa_i64;
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
typedef bool daxa_b32;
typedef bool daxa_b32vec1;
typedef bool2 daxa_b32vec2;
typedef bool3 daxa_b32vec3;
typedef bool4 daxa_b32vec4;

namespace daxa
{
    struct BufferId
    {
        daxa_u64 value;
        daxa_u32 index()
        {
            return daxa_u32(DAXA_ID_INDEX_MASK & value);
        }
        daxa_u64 version()
        {
            return (value >> DAXA_ID_VERSION_OFFSTET);
        }
    };

    struct ImageViewId
    {
        daxa_u64 value;
        daxa_u32 index()
        {
            return daxa_u32(DAXA_ID_INDEX_MASK & value);
        }
        daxa_u64 version()
        {
            return (value >> DAXA_ID_VERSION_OFFSTET);
        }
    };

    struct ImageViewIndex
    {
        daxa_u32 value;
    };

    struct SamplerId
    {
        daxa_u64 value;
        daxa_u32 index()
        {
            return daxa_u32(DAXA_ID_INDEX_MASK & value);
        }
        daxa_u64 version()
        {
            return (value >> DAXA_ID_VERSION_OFFSTET);
        }
    };
} // namespace daxa
#define daxa_BufferId daxa::BufferId
#define daxa_ImageViewId daxa::ImageViewId
#define daxa_SamplerId daxa::SamplerId

namespace daxa 
{
    [[vk::binding(DAXA_SAMPLER_BINDING, 0)]] SamplerState SamplerStateTable[];
    [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] ByteAddressBuffer ByteAddressBufferTable[];
    [[vk::binding(DAXA_STORAGE_BUFFER_BINDING, 0)]] RWByteAddressBuffer RWByteAddressBufferTable[];
}
#define daxa_SamplerState(SAMPLER_ID) SamplerStateTable[SAMPLER_ID.index()]
#define daxa_ByteAddressBuffer(SAMPLER_ID) ByteAddressBufferTable[SAMPLER_ID.index()]
#define daxa_RWByteAddressBuffer(SAMPLER_ID) RWByteAddressBufferTable[SAMPLER_ID.index()]

#define daxa_StructuredBuffer(STRUCT_TYPE, BUFFER_ID) StructuredBuffer##STRUCT_TYPE##Table[BUFFER_ID.index()]

#define _DAXA_DECL_TEXTURE_RET(TYPE, RET_TYPE) \
    namespace daxa { [[vk::binding(DAXA_SAMPLED_IMAGE_BINDING, 0)]] TYPE<RET_TYPE> TYPE##RET_TYPE##Table[]; }
#define _DAXA_DECL_RWTEXTURE_RET(TYPE, RET_TYPE) \
    namespace daxa { [[vk::binding(DAXA_STORAGE_IMAGE_BINDING, 0)]] RW##TYPE<RET_TYPE> RW##TYPE##RET_TYPE##Table[]; }
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
_DAXA_DECL_TEXTURE(Texture1D)
_DAXA_DECL_TEXTURE(Texture1DArray)
_DAXA_DECL_TEXTURE(Texture2D)
_DAXA_DECL_TEXTURE(Texture2DArray)
_DAXA_DECL_TEXTURE(Texture3D)
_DAXA_DECL_TEXTURE_NO_RW(Texture2DMS)
_DAXA_DECL_TEXTURE_NO_RW(TextureCube)
_DAXA_DECL_TEXTURE_NO_RW(TextureCubeArray)

#define daxa_RWTexture1D(RET_TYPE, IMAGE_VIEW_ID) daxa::RWTexture1D##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_RWTexture1DArray(RET_TYPE, IMAGE_VIEW_ID) daxa::RWTexture1DArray##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_RWTexture2D(RET_TYPE, IMAGE_VIEW_ID) daxa::RWTexture2D##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_RWTexture2DArray(RET_TYPE, IMAGE_VIEW_ID) daxa::RWTexture2DArray##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_RWTexture3D(RET_TYPE, IMAGE_VIEW_ID) daxa::RWTexture3D##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_Texture1D(RET_TYPE, IMAGE_VIEW_ID) daxa::Texture1D##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_Texture1DArray(RET_TYPE, IMAGE_VIEW_ID) daxa::Texture1DArray##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_Texture2D(RET_TYPE, IMAGE_VIEW_ID) daxa::Texture2D##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_Texture2DArray(RET_TYPE, IMAGE_VIEW_ID) daxa::Texture2DArray##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_Texture3D(RET_TYPE, IMAGE_VIEW_ID) daxa::Texture3D##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_Texture2DMS(RET_TYPE, IMAGE_VIEW_ID) daxa::Texture2DMS##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_TextureCube(RET_TYPE, IMAGE_VIEW_ID) daxa::TextureCube##RET_TYPE##Table[IMAGE_VIEW_ID.index()]
#define daxa_TextureCubeArray(RET_TYPE, IMAGE_VIEW_ID) daxa::TextureCubeArray##RET_TYPE##Table[IMAGE_VIEW_ID.index()]

namespace daxa
{
    [[vk::binding(DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING, 0)]] StructuredBuffer<daxa_u64> buffer_addresses;
    func id_to_address(daxa::BufferId buffer_id) -> daxa_u64
    {
        return buffer_addresses[buffer_id.index()];
    }
}

#define daxa_BufferPtr(STRUCT_TYPE) daxa_BufferPtr##STRUCT_TYPE
#define daxa_RWBufferPtr(STRUCT_TYPE) daxa_RWBufferPtr##STRUCT_TYPE

#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN) DAXA_DECL_BUFFER_PTR(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE) \
    struct daxa_RWBufferPtr##STRUCT_TYPE  \
    {                                     \
        STRUCT_TYPE* value;               \
    };                                    \
    struct daxa_BufferPtr##STRUCT_TYPE    \
    {                                     \
        STRUCT_TYPE* value;               \
    };

#define deref(PTR) PTR.value