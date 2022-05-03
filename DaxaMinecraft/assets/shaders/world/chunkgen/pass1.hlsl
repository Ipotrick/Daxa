#include "world/common.hlsl"
#include "world/chunkgen/noise.hlsl"

#include "utils/shape_dist.hlsl"

#include "core.hlsl"

uint gen_block(float3 b_pos) {
    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globals_sb);
    int3 chunk_i = p.pos.xyz / CHUNK_SIZE;
    int3 chunk_i_min = int3(max(chunk_i.x - 1, 0), max(chunk_i.y - 3, 0), max(chunk_i.z - 1, 0));
    int3 chunk_i_max = int3(min(chunk_i.x + 1, CHUNK_NX), min(chunk_i.y + 3, CHUNK_NY), min(chunk_i.z + 1, CHUNK_NZ));

    BlockID result;
    result = BlockID::Debug;

    for (uint cz = chunk_i_min.z; cz < chunk_i_max.z; ++cz) {
        for (uint cy = chunk_i_min.y; cy < chunk_i_max.y; ++cy) {
            for (uint cx = chunk_i_min.x; cx < chunk_i_max.x; ++cx) {
                uint structure_n = globals[0].chunkgen_data[cz][cy][cx].structure_n;
                for (uint i = 0; i < min(structure_n, 127); ++i) {
                    // float3 off = b_pos - globals[0].chunkgen_data[cz][cy][cx].structures[i].p.xyz;
                    // if (dot(off, off) < 5 * 5)
                    //     result = BlockID::Brick;

                    switch (globals[0].chunkgen_data[cz][cy][cx].structures[i].id) {
                    case 1: {
                        TreeSDF tree_sdf = sd_tree_spruce(b_pos, 0, globals[0].chunkgen_data[cz][cy][cx].structures[i].p.xyz + float3(0, 1, 0));
                        if (tree_sdf.trunk_dist < 0)
                            result = BlockID::Log;
                        if (tree_sdf.leaves_dist < 0)
                            result = BlockID::Leaves;
                    } break;
                    case 2: {
                        TreeSDF tree_sdf = sd_tree_pine(b_pos, 0, globals[0].chunkgen_data[cz][cy][cx].structures[i].p.xyz + float3(0, 3, 0));
                        if (tree_sdf.trunk_dist < 0)
                            result = BlockID::Log;
                        if (tree_sdf.leaves_dist < 0)
                            result = BlockID::Leaves;
                    } break;
                    }
                }
            }
        }
    }
    return (uint)result;
}

[numthreads(8, 8, 8)] void main(uint3 global_i
                                : SV_DispatchThreadID) {
    float3 block_pos = float3(global_i) + p.pos.xyz;

    uint chunk_texture_id = p.output_image_i;
    RWTexture3D<uint> chunk = daxa::getRWTexture3D<uint>(chunk_texture_id);

    uint new_id = gen_block(block_pos);

    if (new_id == 0)
        new_id = chunk[int3(global_i)];

    chunk[int3(global_i)] = new_id;
}
