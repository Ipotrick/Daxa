#pragma once

namespace daxa
{
    typedef uint u32;
    typedef bool b32;

    typedef int i32;

    typedef float f32;
    typedef double f64;

    typedef bool b32;
    typedef bool2 b32vec2;
    typedef bool3 b32vec3;
    typedef bool4 b32vec4;
    typedef float f32;
    typedef double f64;
    typedef float2 f32vec2;
    typedef float2x2 f32vec2x2;
    typedef float2x3 f32vec2x3;
    typedef float2x4 f32vec2x4;
    typedef double2 f64vec2;
    typedef double2x2 f64vec2x2;
    typedef double2x3 f64vec2x3;
    typedef double2x4 f64vec2x4;
    typedef float3 f32vec3;
    typedef float3x2 f32vec3x2;
    typedef float3x3 f32vec3x3;
    typedef float3x4 f32vec3x4;
    typedef double3 f64vec3;
    typedef double3x2 f64vec3x2;
    typedef double3x3 f64vec3x3;
    typedef double3x4 f64vec3x4;
    typedef float4 f32vec4;
    typedef float4x2 f32vec4x2;
    typedef float4x3 f32vec4x3;
    typedef float4x4 f32vec4x4;
    typedef double4 f64vec4;
    typedef double4x2 f64vec4x2;
    typedef double4x3 f64vec4x3;
    typedef double4x4 f64vec4x4;
    typedef float4 f32vec4;
    typedef float4x2 f32vec4x2;
    typedef float4x3 f32vec4x3;
    typedef float4x4 f32vec4x4;
    typedef int i32;
    typedef uint u32;
    typedef int2 i32vec2;
    typedef int2x2 i32vec2x2;
    typedef int2x3 i32vec2x3;
    typedef int2x4 i32vec2x4;
    typedef uint2 u32vec2;
    typedef uint2x2 u32vec2x2;
    typedef uint2x3 u32vec2x3;
    typedef uint2x4 u32vec2x4;
    typedef int3 i32vec3;
    typedef int3x2 i32vec3x2;
    typedef int3x3 i32vec3x3;
    typedef int3x4 i32vec3x4;
    typedef uint3 u32vec3;
    typedef uint3x2 u32vec3x2;
    typedef uint3x3 u32vec3x3;
    typedef uint3x4 u32vec3x4;
    typedef int4 i32vec4;
    typedef int4x2 i32vec4x2;
    typedef int4x3 i32vec4x3;
    typedef int4x4 i32vec4x4;
    typedef uint4 u32vec4;
    typedef uint4x2 u32vec4x2;
    typedef uint4x3 u32vec4x3;
    typedef uint4x4 u32vec4x4;

    enum CONSTANTS
    {
        STORAGE_BUFFER_BINDING = 0,
        STORAGE_IMAGE_BINDING = 1,
        SAMPLED_IMAGE_BINDING = 2,
        SAMPLER_BINDING = 3,
        ID_INDEX_MASK = 0xFFFFFFFF >> 8,
    };

    struct BufferId
    {
        uint data;
    };

    struct ImageId
    {
        uint data;
    };

    struct SamplerId
    {
        uint data;
    };

    template <typename T>
    Buffer<T> get_Buffer(BufferId buffer_id);
    template <typename T>
    StructuredBuffer<T> get_StructuredBuffer(BufferId buffer_id);
    template <typename T>
    Texture1D<T> get_Texture1D(ImageId image_id);
    template <typename T>
    Texture2D<T> get_Texture2D(ImageId image_id);
    template <typename T>
    Texture3D<T> get_Texture3D(ImageId image_id);
    template <typename T>
    Texture1DArray<T> get_Texture1DArray(ImageId image_id);
    template <typename T>
    Texture2DArray<T> get_Texture2DArray(ImageId image_id);
    template <typename T>
    RWTexture1D<T> get_RWTexture1D(ImageId image_id);
    template <typename T>
    RWTexture2D<T> get_RWTexture2D(ImageId image_id);
    template <typename T>
    RWTexture3D<T> get_RWTexture3D(ImageId image_id);

    [[vk::binding(daxa::CONSTANTS::SAMPLER_BINDING, 0)]] SamplerState SamplerStateView[];
    SamplerState get_sampler(SamplerId sampler_id)
    {
        return SamplerStateView[ID_INDEX_MASK & sampler_id.data];
    }

} // namespace daxa

#define DAXA_DEFINE_GET_STRUCTURED_BUFFER(Type)                                                                          \
    namespace daxa                                                                                                       \
    {                                                                                                                    \
        [[vk::binding(daxa::CONSTANTS::STORAGE_BUFFER_BINDING, 0)]] StructuredBuffer<Type> StructuredBufferView##Type[]; \
        template <>                                                                                                      \
        StructuredBuffer<Type> get_StructuredBuffer(BufferId buffer_id)                                                  \
        {                                                                                                                \
            return StructuredBufferView##Type[ID_INDEX_MASK & buffer_id.data];                                           \
        }                                                                                                                \
    }
#define DAXA_DEFINE_GET_BUFFER(Type)                                                                 \
    namespace daxa                                                                                   \
    {                                                                                                \
        [[vk::binding(daxa::CONSTANTS::STORAGE_BUFFER_BINDING, 0)]] Buffer<Type> BufferView##Type[]; \
        template <>                                                                                  \
        Buffer<Type> get_Buffer(BufferId buffer_id)                                                  \
        {                                                                                            \
            return BufferView##Type[ID_INDEX_MASK & buffer_id.data];                                 \
        }                                                                                            \
    }
#define DAXA_DEFINE_GET_TEXTURE1D(Type)                                                                   \
    namespace daxa                                                                                        \
    {                                                                                                     \
        [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture2D<Type> Texture1DView##Type[]; \
        template <>                                                                                       \
        Texture1D<Type> get_Texture1D<Type>(ImageId image_id)                                             \
        {                                                                                                 \
            return Texture1DView##Type[ID_INDEX_MASK & image_id.data];                                    \
        }                                                                                                 \
    }
#define DAXA_DEFINE_GET_TEXTURE2D(Type)                                                                   \
    namespace daxa                                                                                        \
    {                                                                                                     \
        [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture2D<Type> Texture2DView##Type[]; \
        template <>                                                                                       \
        Texture2D<Type> get_Texture2D<Type>(ImageId image_id)                                             \
        {                                                                                                 \
            return Texture2DView##Type[ID_INDEX_MASK & image_id.data];                                    \
        }                                                                                                 \
    }
#define DAXA_DEFINE_GET_TEXTURE3D(Type)                                                                   \
    namespace daxa                                                                                        \
    {                                                                                                     \
        [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture3D<Type> Texture3DView##Type[]; \
        template <>                                                                                       \
        Texture3D<Type> get_Texture3D<Type>(ImageId image_id)                                             \
        {                                                                                                 \
            return Texture3DView##Type[ID_INDEX_MASK & image_id.data];                                    \
        }                                                                                                 \
    }
#define DAXA_DEFINE_GET_TEXTURE1DARRAY(Type)                                                                        \
    namespace daxa                                                                                                  \
    {                                                                                                               \
        [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture1DArray<Type> Texture1DArrayView##Type[]; \
        template <>                                                                                                 \
        Texture1DArray<Type> get_Texture1DArray<Type>(ImageId image_id)                                             \
        {                                                                                                           \
            return Texture1DArrayView##Type[ID_INDEX_MASK & image_id.data];                                         \
        }                                                                                                           \
    }
#define DAXA_DEFINE_GET_TEXTURE2DARRAY(Type)                                                                        \
    namespace daxa                                                                                                  \
    {                                                                                                               \
        [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture2DArray<Type> Texture2DArrayView##Type[]; \
        template <>                                                                                                 \
        Texture2DArray<Type> get_Texture2DArray<Type>(ImageId image_id)                                             \
        {                                                                                                           \
            return Texture2DArrayView##Type[ID_INDEX_MASK & image_id.data];                                         \
        }                                                                                                           \
    }
#define DAXA_DEFINE_GET_RWTEXTURE1D(Type)                                                                     \
    namespace daxa                                                                                            \
    {                                                                                                         \
        [[vk::binding(daxa::CONSTANTS::STORAGE_IMAGE_BINDING, 0)]] RWTexture1D<Type> RWTexture1DView##Type[]; \
        template <>                                                                                           \
        RWTexture1D<Type> get_RWTexture1D<Type>(ImageId image_id)                                             \
        {                                                                                                     \
            return RWTexture1DView##Type[ID_INDEX_MASK & image_id.data];                                      \
        }                                                                                                     \
    }
#define DAXA_DEFINE_GET_RWTEXTURE2D(Type)                                                                     \
    namespace daxa                                                                                            \
    {                                                                                                         \
        [[vk::binding(daxa::CONSTANTS::STORAGE_IMAGE_BINDING, 0)]] RWTexture2D<Type> RWTexture2DView##Type[]; \
        template <>                                                                                           \
        RWTexture2D<Type> get_RWTexture2D<Type>(ImageId image_id)                                             \
        {                                                                                                     \
            return RWTexture2DView##Type[ID_INDEX_MASK & image_id.data];                                      \
        }                                                                                                     \
    }
#define DAXA_DEFINE_GET_RWTEXTURE3D(Type)                                                                     \
    namespace daxa                                                                                            \
    {                                                                                                         \
        [[vk::binding(daxa::CONSTANTS::STORAGE_IMAGE_BINDING, 0)]] RWTexture3D<Type> RWTexture3DView##Type[]; \
        template <>                                                                                           \
        RWTexture3D<Type> get_RWTexture3D<Type>(ImageId image_id)                                             \
        {                                                                                                     \
            return RWTexture3DView##Type[ID_INDEX_MASK & image_id.data];                                      \
        }                                                                                                     \
    }

DAXA_DEFINE_GET_TEXTURE2D(float)
DAXA_DEFINE_GET_TEXTURE2D(float2)
DAXA_DEFINE_GET_TEXTURE2D(float4)
DAXA_DEFINE_GET_TEXTURE2D(uint)
DAXA_DEFINE_GET_TEXTURE2D(uint4)

DAXA_DEFINE_GET_RWTEXTURE2D(float)
DAXA_DEFINE_GET_RWTEXTURE2D(float2)
DAXA_DEFINE_GET_RWTEXTURE2D(float4)
DAXA_DEFINE_GET_RWTEXTURE2D(uint)
DAXA_DEFINE_GET_RWTEXTURE2D(uint4)
