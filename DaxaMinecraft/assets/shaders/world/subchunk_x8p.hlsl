#include "chunk.hlsl"

struct Push {
    uint4 chunk_i;
    uint globalsID;
    uint mode;
};
[[vk::push_constant]] const Push p;

groupshared uint local_x8_copy[64];
groupshared uint local_x16_copy[16];

#define Log2_x4 2
#define Log2_x8 3
#define Log2_x16 4
#define Log2_x32 5

/*
    x: in workgroup index
    y: in chunk index
    z: chunk index
*/
[numthreads(512,1,1)]
void Main(
    uint3 group_local_ID : SV_GroupThreadID
) {
    uint3 x8_i = uint3(
        (group_local_ID.x >> 3) & 0x7,
        (group_local_ID.x >> 6) & 0x7,
        group_local_ID.x & 0x7
    );
    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globalsID);
    uint3 chunk_i = p.chunk_i.xyz;
    if (p.mode == 1) {
        chunk_i += int3(globals[0].pick_pos[0].xyz) / CHUNK_SIZE;
    }
    if (chunk_i.x < 0 || chunk_i.x >= CHUNK_NX ||
        chunk_i.y < 0 || chunk_i.y >= CHUNK_NY ||
        chunk_i.z < 0 || chunk_i.z >= CHUNK_NZ)
        return;
    uint chunk_id = globals[0].chunk_images[chunk_i.z][chunk_i.y][chunk_i.x];
    if (chunk_id == globals[0].empty_chunk_index)
        return;
    RWTexture3D<uint> chunk = daxa::getRWTexture3D<uint>(chunk_id);
    uint3 x4_i = x8_i * 2;

    bool at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        int3 local_i = x4_i + int3(x,y,z);
        uint index = x_index<4>(local_i);
        uint mask = x_mask(local_i);
        bool occluding = (globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x].x4[index] & mask) != 0;
        at_least_one_occluding = at_least_one_occluding || occluding;
    }

    uint result = 0;
    if (at_least_one_occluding) {
        result = x_mask(x8_i);
    }
    for (int i = 0; i < 4; i++) {
        if ((WaveGetLaneIndex() >> 3) == i) {
            result = WaveActiveBitOr(result);
        }
    }
    if ((WaveGetLaneIndex() & 0x7 /* == % 8*/) == 0) {
        uint index = x_index<8>(x8_i);
        globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x].x8[index] = result;
        local_x8_copy[index] = result;
    }

    GroupMemoryBarrierWithGroupSync();

    if (group_local_ID.x >= 64) {
        return;
    }

    uint3 x16_i = uint3(
        (group_local_ID.x >> 2) & 0x3, 
        (group_local_ID.x >> 4) & 0x3, 
        group_local_ID.x & 0x3
    );
    x8_i = x16_i * 2;

    at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        int3 local_i = x8_i + int3(x,y,z);
        uint mask = x_mask(local_i);
        uint index = x_index<8>(local_i);
        bool is_occluding = (local_x8_copy[index] & mask) != 0;
        at_least_one_occluding = at_least_one_occluding || is_occluding;
    }

    result = 0;
    if (at_least_one_occluding) {
        result = x_mask(x16_i);
    }
    for (int i = 0; i < 8; i++) {
        if ((WaveGetLaneIndex() >> 2) == i) {
            result = WaveActiveBitOr(result);
        }
    }
    if ((WaveGetLaneIndex() & 0x3) == 0) {
        uint index = x_index<16>(x16_i);
        globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x].x16[index] = result;
        local_x16_copy[index] = result;
    }

    GroupMemoryBarrierWithGroupSync();

    if (group_local_ID.x >= 8) {
        return;
    }

    uint3 x32_i = uint3(
        (group_local_ID.x >> 1) & 0x1,
        (group_local_ID.x >> 2) & 0x1,
        group_local_ID.x & 0x1
    );
    x16_i = x32_i * 2;

    at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        int3 local_i = x16_i + int3(x,y,z);
        uint mask = x_mask(local_i);
        uint index = x_index<16>(local_i);
        bool is_occluding = (local_x16_copy[index] & mask) != 0;
        at_least_one_occluding = at_least_one_occluding || is_occluding;
    }

    result = 0;
    if (at_least_one_occluding) {
        result = x_mask(x32_i);
    }
    for (int i = 0; i < 16; i++) {
        if ((WaveGetLaneIndex() >> 1) == i) {
            result = WaveActiveBitOr(result);
        }
    }
    if ((WaveGetLaneIndex() & 0x1) == 0) {
        uint index = x_index<32>(x32_i);
        globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x].x32[index] = result;
    }
}