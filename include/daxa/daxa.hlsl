#pragma once

namespace daxa
{
    enum CONSTANTS
    {
        STORAGE_BUFFER_BINDING = 0,
        STORAGE_IMAGE_BINDING = 1,
        SAMPLED_IMAGE_BINDING = 2,
        SAMPLER_BINDING = 3,
        ID_INDEX_MASK = 0x00FFFFFF,
    };

    struct BufferId
    {
        uint data;
    };

    struct ImageId
    {
        uint data;
    };

    // using ImageViewId = ImageId;

    struct SamplerId
    {
        uint data;
    };

    template <typename T>
    StructuredBuffer<T> get_Buffer(BufferId buffer_id);
    template <typename T>
    StructuredBuffer<T> get_RWBuffer(BufferId buffer_id);
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

#define DAXA_DEFINE_GET_BUFFER(Type)                                                                           \
    namespace daxa                                                                                             \
    {                                                                                                          \
        [[vk::binding(daxa::CONSTANTS::STORAGE_BUFFER_BINDING, 0)]] StructuredBuffer<Type> BufferView##Type[]; \
        template <>                                                                                            \
        StructuredBuffer<Type> get_Buffer(BufferId buffer_id)                                                  \
        {                                                                                                      \
            return BufferView##Type[ID_INDEX_MASK & buffer_id.data];                                           \
        }                                                                                                      \
    }
#define DAXA_DEFINE_GET_RWBUFFER(Type)                                                                              \
    namespace daxa                                                                                                  \
    {                                                                                                               \
        [[vk::binding(daxa::CONSTANTS::STORAGE_BUFFER_BINDING, 0)]] RWStructuredBuffer<Type> RWBufferView##Type[];  \
        template <>                                                                                                 \
        RWStructuredBuffer<Type> get_RWBuffer(BufferId buffer_id)                                                   \
        {                                                                                                           \
            return RWBufferView##Type[ID_INDEX_MASK & buffer_id.data];                                              \
        }                                                                                                           \
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
