#pragma once

namespace daxa {
    enum CONSTANTS{
        SAMPLER_BINDING = 0,
        SAMPLED_IMAGE_BINDING = 2,
        STORAGE_IMAGE_BINDING = 3,
        STORAGE_BUFFER_BINDING = 4,
    };

    template <typename T>
    SamplerState getSampler(uint id);
    template <typename T>
    Texture2D<T> getTexture2D(uint id);
    template <typename T>
    Texture2DArray<T> getTexture2DArray(uint id);
    template <typename T>
    Texture3D<T> getTexture3D(uint id);
    template <typename T>
    RWTexture2D<T> getRWTexture2D(uint id);
    template <typename T>
    RWTexture3D<T> getRWTexture3D(uint id);
    template <typename T>
    StructuredBuffer<T> getBuffer(uint id);
}

#define DAXA_DEFINE_BA_SAMPLER(Type)                                \
namespace daxa {                                                    \
    [[vk::binding(daxa::CONSTANTS::SAMPLER_BINDING, 0)]] SamplerState SamplerStateView##Type[]; \
    template <>                                                     \
    SamplerState getSampler<Type>(uint id) {                        \
        return SamplerStateView##Type[id];                          \
    }                                                                \
}
#define DAXA_DEFINE_BA_TEXTURE2D(Type)                              \
namespace daxa {                                                    \
    [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture2D<Type> Texture2DView##Type[]; \
    template <>                                                     \
    Texture2D<Type> getTexture2D<Type>(uint id) {                   \
        return Texture2DView##Type[id];                             \
    }                                                                \
}
#define DAXA_DEFINE_BA_TEXTURE2DARRAY(Type)                         \
namespace daxa {                                                    \
    [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture2DArray<Type> Texture2DArrayView##Type[]; \
    template <>                                                     \
    Texture2DArray<Type> getTexture2DArray<Type>(uint id) {         \
        return Texture2DArrayView##Type[id];                        \
    }                                                                \
}
#define DAXA_DEFINE_BA_TEXTURE3D(Type)                              \
namespace daxa {                                                    \
    [[vk::binding(daxa::CONSTANTS::SAMPLED_IMAGE_BINDING, 0)]] Texture3D<Type> Texture3DView##Type[]; \
    template <>                                                     \
    Texture3D<Type> getTexture3D<Type>(uint id) {                   \
        return Texture3DView##Type[id];                             \
    }                                                                \
}
#define DAXA_DEFINE_BA_RWTEXTURE2D(Type)                             \
namespace daxa {                                                    \
    [[vk::binding(daxa::CONSTANTS::STORAGE_IMAGE_BINDING, 0)]] RWTexture2D<Type> RWTexture2DView##Type[]; \
    template <>                                                      \
    RWTexture2D<Type> getRWTexture2D<Type>(uint id) {                \
        return RWTexture2DView##Type[id];                            \
    }                                                                \
}
#define DAXA_DEFINE_BA_RWTEXTURE3D(Type)                             \
namespace daxa {                                                    \
    [[vk::binding(daxa::CONSTANTS::STORAGE_IMAGE_BINDING, 0)]] RWTexture3D<Type> RWTexture3DView##Type[]; \
    template <>                                                  	\
    RWTexture3D<Type> getRWTexture3D<Type>(uint id) {            	\
        return RWTexture3DView##Type[id];                        	\
    }                                                                \
}
#define DAXA_DEFINE_BA_BUFFER(Type)									\
namespace daxa {													\
    [[vk::binding(daxa::CONSTANTS::STORAGE_BUFFER_BINDING, 0)]] StructuredBuffer<Type> BufferView##Type[]; \
    template <>                                                      \
    StructuredBuffer<Type> getBuffer(uint id) {                      \
        return BufferView##Type[id];                                 \
    }                                                                \
}

DAXA_DEFINE_BA_TEXTURE2D(float)
DAXA_DEFINE_BA_TEXTURE2D(float2)
DAXA_DEFINE_BA_TEXTURE2D(float4)

DAXA_DEFINE_BA_RWTEXTURE2D(float)
DAXA_DEFINE_BA_RWTEXTURE2D(float2)
DAXA_DEFINE_BA_RWTEXTURE2D(float4)