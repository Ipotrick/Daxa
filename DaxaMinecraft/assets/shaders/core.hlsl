#pragma once

#include "block_info.hlsl"

template <typename T>
StructuredBuffer<T> getBuffer(uint id);
template <typename T>
SamplerState getSampler(uint id);
template <typename T>
Texture2D<T> getTexture2D(uint id);
template <typename T>
RWTexture2D<T> getRWTexture2D(uint id);
template <typename T>
Texture2DArray<T> getTexture2DArray(uint id);
template <typename T>
Texture3D<T> getTexture3D(uint id);
template <typename T>
RWTexture3D<T> getRWTexture3D(uint id);

#define DAXA_DEFINE_BA_BUFFER(Type)                                  \
    [[vk::binding(4, 0)]] StructuredBuffer<Type> BufferView##Type[]; \
    template <>                                                      \
    StructuredBuffer<Type> getBuffer(uint id) {                      \
        return BufferView##Type[id];                                 \
    }
#define DAXA_DEFINE_BA_SAMPLER(Type)                             \
    [[vk::binding(1, 0)]] SamplerState SamplerStateView##Type[]; \
    template <>                                                  \
    SamplerState getSampler<Type>(uint id) {                     \
        return SamplerStateView##Type[id];                       \
    }
#define DAXA_DEFINE_BA_RWTEXTURE2D(Type)                             \
    [[vk::binding(3, 0)]] RWTexture2D<Type> RWTexture2DView##Type[]; \
    template <>                                                      \
    RWTexture2D<Type> getRWTexture2D<Type>(uint id) {                \
        return RWTexture2DView##Type[id];                            \
    }
#define DAXA_DEFINE_BA_TEXTURE2DARRAY(Type)                                \
    [[vk::binding(1, 0)]] Texture2DArray<Type> Texture2DArrayView##Type[]; \
    template <>                                                            \
    Texture2DArray<Type> getTexture2DArray<Type>(uint id) {                \
        return Texture2DArrayView##Type[id];                               \
    }
#define DAXA_DEFINE_BA_TEXTURE3D(Type)                           \
    [[vk::binding(1, 0)]] Texture3D<Type> Texture3DView##Type[]; \
    template <>                                                  \
    Texture3D<Type> getTexture3D<Type>(uint id) {                \
        return Texture3DView##Type[id];                          \
    }
#define DAXA_DEFINE_BA_RWTEXTURE3D(Type)                             \
    [[vk::binding(3, 0)]] RWTexture3D<Type> RWTexture3DView##Type[]; \
    template <>                                                      \
    RWTexture3D<Type> getRWTexture3D<Type>(uint id) {                \
        return RWTexture3DView##Type[id];                            \
    }

DAXA_DEFINE_BA_RWTEXTURE2D(float4)
DAXA_DEFINE_BA_RWTEXTURE3D(uint)

struct ChunkBlockPresence {
    uint x2[1024];
    uint x4[256];
    uint x8[64];
    uint x16[16];
    uint x32[4];
};

struct Globals {
    float4x4 viewproj_mat;
    float4 pos;
    float4 single_ray_pos;
    float4 single_ray_nrm;
    float4 pick_pos[1];
    int2 frame_dim;
    float time, fov;
    uint texture_index;
    uint empty_chunk_index;
    uint model_load_index;
    uint single_ray_steps;
    uint chunk_images[CHUNK_NZ * CHUNK_INDEX_REPEAT_Z][CHUNK_NY * CHUNK_INDEX_REPEAT_Y][CHUNK_NX * CHUNK_INDEX_REPEAT_X];
    ChunkBlockPresence chunk_block_presence[CHUNK_NZ][CHUNK_NY][CHUNK_NX];
};

struct ModelLoadBuffer {
    float4 pos, dim;
    uint data[128 * 128 * 128];
};

DAXA_DEFINE_BA_BUFFER(Globals)
DAXA_DEFINE_BA_BUFFER(ModelLoadBuffer)

#if !defined(USE_GLOBALS_DEFINE)
#define USE_GLOBALS_DEFINE 0
#endif

#if USE_GLOBALS_DEFINE
#define GLOBALS_DEFINE getBuffer<Globals>(p.globals_sb)[0]
#define GLOBALS_ARG
#define GLOBALS_PARAM
#else
#define GLOBALS_DEFINE globals[0]
#define GLOBALS_ARG globals,
#define GLOBALS_PARAM StructuredBuffer<Globals> globals,
#endif

BlockID load_block_id(GLOBALS_PARAM float3 pos) {
    int3 chunk_i = int3(pos / CHUNK_SIZE);
    if (chunk_i.x < 0 || chunk_i.x > CHUNK_NX * CHUNK_INDEX_REPEAT_X - 1 ||
        chunk_i.y < 0 || chunk_i.y > CHUNK_NY * CHUNK_INDEX_REPEAT_Y - 1 ||
        chunk_i.z < 0 || chunk_i.z > CHUNK_NZ * CHUNK_INDEX_REPEAT_Z - 1) {
        return BlockID::Air;
    }
    return (BlockID)getRWTexture3D<uint>(GLOBALS_DEFINE.chunk_images[chunk_i.z][chunk_i.y][chunk_i.x])
        [int3(pos) - chunk_i * int3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)];
}
