template <typename T>
RWTexture2D<T> getRWTexture2D(uint id);
template <typename T>
StructuredBuffer<T> getBuffer(uint id);

#define DAXA_DEFINE_BA_BUFFER(Type)                                  \
    [[vk::binding(4, 0)]] StructuredBuffer<Type> BufferView##Type[]; \
    template <>                                                      \
    StructuredBuffer<Type> getBuffer(uint id) {                      \
        return BufferView##Type[id];                                 \
    }
#define DAXA_DEFINE_BA_RWTEXTURE2D(Type)                             \
    [[vk::binding(3, 0)]] RWTexture2D<Type> RWTexture2DView##Type[]; \
    template <>                                                      \
    RWTexture2D<Type> getRWTexture2D<Type>(uint id) {                \
        return RWTexture2DView##Type[id];                            \
    }

DAXA_DEFINE_BA_RWTEXTURE2D(float4)

struct Globals {
    int2 frame_dim;
    float time;
};
DAXA_DEFINE_BA_BUFFER(Globals)

struct Push {
    uint globals_id;
    uint output_image_id;
};

[[vk::push_constant]] const Push p;

#define SHADERTOY_NUMTHREADS [numthreads(8, 8, 1)]

#include "user.hlsl"
