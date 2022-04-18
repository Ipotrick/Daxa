#pragma once

#include "core.hlsl"

// Log2_N = 1 : x2, 2 : x4, 3: x8 ...
template <uint N>
uint x_index(uint3 x_i) {
    enum CONSTANTS : uint {
        STRIDE = 64 / N,
    };
    return x_i.x + x_i.y * uint(64 / N);
}

uint x_mask(uint3 x_i) {
    return 1u << x_i.z;
}

template<uint N>
bool x_presence_in_chunk(ChunkBlockPresence chunk_presence, uint index, uint mask) {
    return true;
}

#define DAXA_IMPLEMENT_X_PRESENCE_IN_CHUNK(N)\
template<>\
bool x_presence_in_chunk<N>(ChunkBlockPresence chunk_presence, uint index, uint mask) {\
    return (chunk_presence.x##N[index] & mask) != 0;\
}

DAXA_IMPLEMENT_X_PRESENCE_IN_CHUNK(2)
DAXA_IMPLEMENT_X_PRESENCE_IN_CHUNK(4)
DAXA_IMPLEMENT_X_PRESENCE_IN_CHUNK(8)
DAXA_IMPLEMENT_X_PRESENCE_IN_CHUNK(16)
DAXA_IMPLEMENT_X_PRESENCE_IN_CHUNK(32)

template<uint N>
bool x_load_presence(StructuredBuffer<Globals> globals, float3 world_pos) {
    uint3 chunk_i = int3(world_pos / CHUNK_SIZE);
    uint3 in_chunk_p = int3(world_pos) - chunk_i * CHUNK_SIZE;
    uint3 x_i = in_chunk_p / N;
    uint index = x_index<N>(x_i);
    uint mask = x_mask(x_i);
    return x_presence_in_chunk<N>(
        globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x],
        index,
        mask);
}

template<>
bool x_load_presence<64>(StructuredBuffer<Globals> globals, float3 world_pos) {
    uint3 chunk_i = int3(world_pos / CHUNK_SIZE);
    uint3 in_chunk_p = int3(world_pos) - chunk_i * CHUNK_SIZE;
    uint3 x_i = in_chunk_p / 64;
    uint x32_local_copy[4] = globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x].x32;
    return (
        x32_local_copy[0] |
        x32_local_copy[1] |
        x32_local_copy[2] |
        x32_local_copy[3]
    ) != 0;
}
