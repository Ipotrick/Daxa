#pragma once

#include "block_info.hlsl"
#include "daxa.hlsl"

DAXA_DEFINE_BA_RWTEXTURE3D(uint)
DAXA_DEFINE_BA_TEXTURE3D(uint)

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
    float4 pick_pos[2];
    int2 frame_dim;
    float time, fov;

    uint texture_index;
    uint empty_chunk_index;
    uint model_load_index;
    uint chunk_images[CHUNK_NZ * CHUNK_INDEX_REPEAT_Z][CHUNK_NY * CHUNK_INDEX_REPEAT_Y][CHUNK_NX * CHUNK_INDEX_REPEAT_X];

    ChunkBlockPresence chunk_block_presence[CHUNK_NZ][CHUNK_NY][CHUNK_NX];
};

struct ModelLoadBuffer {
    float4 pos, dim;
    uint data[128 * 128 * 128];
};

DAXA_DEFINE_BA_BUFFER(Globals)
DAXA_DEFINE_BA_BUFFER(ModelLoadBuffer)

BlockID load_block_id(StructuredBuffer<Globals> globals, float3 pos) {
    int3 chunk_i = int3(pos / CHUNK_SIZE);
    if (chunk_i.x < 0 || chunk_i.x > CHUNK_NX * CHUNK_INDEX_REPEAT_X - 1 ||
        chunk_i.y < 0 || chunk_i.y > CHUNK_NY * CHUNK_INDEX_REPEAT_Y - 1 ||
        chunk_i.z < 0 || chunk_i.z > CHUNK_NZ * CHUNK_INDEX_REPEAT_Z - 1) {
        return BlockID::Air;
    }
    return (BlockID)daxa::getRWTexture3D<uint>(globals[0].chunk_images[chunk_i.z][chunk_i.y][chunk_i.x])
        [int3(pos) - chunk_i * int3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)];
}
