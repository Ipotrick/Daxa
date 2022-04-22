#include "chunk.hlsl"

struct Push {
    uint4 chunk_i;
    uint globalsID;
    uint mode;
};
[[vk::push_constant]] const Push p;

groupshared uint local_x2_copy[4][4];

#define Log2_x2 1
#define Log2_x4 2

/*
    x: in workgroup index
    y: in chunk index
    z: chunk index
*/
[numthreads(512,1,1)]
void Main(
    uint3 group_local_ID : SV_GroupThreadID,
    uint3 group_ID : SV_GroupID
) {
    uint2 x2_in_group_location = uint2(
        (group_local_ID.x >> 5) & 0x3, 
        (group_local_ID.x >> 7) & 0x3
    );
    uint3 x2_i = uint3(
        (group_local_ID.x >> 5) & 0x3, 
        (group_local_ID.x >> 7) & 0x3, 
        (group_local_ID.x & 0x1F)
    );
    x2_i += 4 * uint3(
        group_ID.y & 0x7,
        (group_ID.y >> 3) & 0x7,
        0
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
    uint3 in_chunk_i = x2_i * 2;

    bool at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        int3 local_i = in_chunk_i + int3(x,y,z);
        at_least_one_occluding = at_least_one_occluding || is_block_occluding((BlockID)chunk[local_i]);
    }

    uint result = 0;
    if (at_least_one_occluding) {
        result = x_mask(x2_i);
    }
    uint or_result = WaveActiveBitOr(result);
    if (WaveIsFirstLane()) {
        uint index = x_index<2>(x2_i);
        globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x].x2[index] = or_result;
        local_x2_copy[x2_in_group_location.x][x2_in_group_location.y] = or_result;
    }

    GroupMemoryBarrierWithGroupSync();

    if (group_local_ID.x >= 64) {
        return;
    }

    uint3 x4_i = uint3(
        (group_local_ID.x >> 4) & 0x1, 
        (group_local_ID.x >> 5) & 0x1, 
        group_local_ID.x & 0xF
    );
    x4_i += 2 * uint3(
        group_ID.y & 0x7,
        (group_ID.y >> 3) & 0x7,
        0
    );
    x2_i = x4_i * 2;
    at_least_one_occluding = false;
    for (int x = 0; x < 2; ++x) 
    for (int y = 0; y < 2; ++y) 
    for (int z = 0; z < 2; ++z) {
        int3 local_i = x2_i + int3(x,y,z);
        uint mask = x_mask(local_i);
        uint2 x2_in_group_index = uint2(
            local_i.x & 0x3,
            local_i.y & 0x3
        );
        bool is_occluding = (local_x2_copy[x2_in_group_index.x][x2_in_group_index.y] & mask) != 0;
        at_least_one_occluding = at_least_one_occluding || is_occluding;
    }
    result = 0;
    if (at_least_one_occluding) {
        result = x_mask(x4_i);
    }
    for (int i = 0; i < 2; i++) {
        if ((WaveGetLaneIndex() >> 4) == i) {
            result = WaveActiveBitOr(result);
        }
    }
    if ((WaveGetLaneIndex() & 0xF /* = %16 */) == 0) {
        uint index = x_index<4>(x4_i);
        globals[0].chunk_block_presence[chunk_i.z][chunk_i.y][chunk_i.x].x4[index] = result;
    }
}