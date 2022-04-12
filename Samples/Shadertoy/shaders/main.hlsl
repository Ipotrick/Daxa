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
    float2 mouse_pos;
    float2 lmb_pos;
    float2 rmb_pos;
    float2 mmb_pos;

    int2 frame_dim;
    float time;

    bool lmb_pressed;
    bool rmb_pressed;
    bool mmb_pressed;
};
DAXA_DEFINE_BA_BUFFER(Globals)

struct Buf0 {
    uint data[128 * 128];
    uint data_1[64 * 64];
    uint data_2[32 * 32];
    uint data_3[16 * 16];
    uint data_4[8 * 8];
    uint data_5[4 * 4];
    uint data_6[2 * 2];
    uint data_7[1 * 1];
};
DAXA_DEFINE_BA_BUFFER(Buf0)

struct Push {
    uint globals_id;
    uint buf0_id;
    uint output_image_id;
};

[[vk::push_constant]] const Push p;

#define SHADERTOY_NUMTHREADS_MAIN [numthreads(8, 8, 1)]
#define SHADERTOY_NUMTHREADS_BUF0 [numthreads(8, 8, 1)]

#include "user.hlsl"
