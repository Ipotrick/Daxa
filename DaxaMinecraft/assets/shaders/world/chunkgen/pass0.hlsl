#include "world/common.hlsl"
#include "world/chunkgen/noise.hlsl"

#include "core.hlsl"

#define water_level 64
#define lava_level 458

void biome_pass0(in out WorldgenState worldgen_state, in float3 b_pos) {
    worldgen_state.biome_id = BiomeID::Plains;
    if (worldgen_state.b_noise < 0.7) {
        worldgen_state.biome_id = BiomeID::Forest;
    } else if (worldgen_state.b_noise > 1.6) {
        worldgen_state.biome_id = BiomeID::Desert;
    }
    if (b_pos.y - water_level > -0.2 + worldgen_state.r * 1.5 &&
        b_pos.y - water_level < 1 + worldgen_state.r * 2 &&
        worldgen_state.t_noise < 0.05 + worldgen_state.r * 0.1 &&
        worldgen_state.t_noise > -0.05 - worldgen_state.r * 0.1) {
        worldgen_state.biome_id = BiomeID::Beach;
    }
}

void block_pass0(in out WorldgenState worldgen_state, in float3 b_pos) {
    worldgen_state.block_id = BlockID::Air;
    if (worldgen_state.t_noise > 0) {
        worldgen_state.block_id = BlockID::Stone;
    } else if (b_pos.y > water_level) {
        worldgen_state.block_id = BlockID::Water;
    }
}

struct SurroundingInfo {
    BlockID above_ids[15];
    BlockID below_ids[15];
    uint depth_above;
    uint depth_below;
    bool above_water, under_water;
};

SurroundingInfo get_surrounding(in out WorldgenState worldgen_state,
                                in float3 b_pos) {
    SurroundingInfo result;

    for (int i = 0; i < 15; ++i) {
        WorldgenState temp;
        float3 sample_pos;

        sample_pos = b_pos + float3(0, i + 1, 0);
        temp = get_worldgen_state(sample_pos);
        block_pass0(temp, sample_pos);
        result.below_ids[i] = temp.block_id;

        sample_pos = b_pos + float3(0, -i - 1, 0);
        temp = get_worldgen_state(sample_pos);
        block_pass0(temp, sample_pos);
        result.above_ids[i] = temp.block_id;
    }

    result.depth_above = 0;
    result.depth_below = 0;
    result.above_water = false;
    result.under_water = false;

    if (worldgen_state.block_id == BlockID::Air) {
        for (; result.depth_above < 15; ++result.depth_above) {
            if (result.above_ids[result.depth_above] == BlockID::Water)
                result.under_water = true;
            if (!is_transparent(result.above_ids[result.depth_above]))
                break;
        }
        for (; result.depth_below < 15; ++result.depth_below) {
            if (result.below_ids[result.depth_below] == BlockID::Water)
                result.above_water = true;
            if (!is_transparent(result.below_ids[result.depth_below]))
                break;
        }
    } else {
        for (; result.depth_above < 15; ++result.depth_above) {
            if (result.above_ids[result.depth_above] == BlockID::Water)
                result.under_water = true;
            if (is_transparent(result.above_ids[result.depth_above]))
                break;
        }
        for (; result.depth_below < 15; ++result.depth_below) {
            if (result.below_ids[result.depth_below] == BlockID::Water)
                result.above_water = true;
            if (is_transparent(result.below_ids[result.depth_below]))
                break;
        }
    }

    return result;
}

void block_pass2(in out WorldgenState worldgen_state, in float3 b_pos,
                 in SurroundingInfo surroundings) {
    if (!is_transparent(worldgen_state.block_id)) {
        switch (worldgen_state.biome_id) {
        case BiomeID::Beach:
            worldgen_state.block_id = BlockID::Sand;
            break;
        case BiomeID::Caves:
            break;
        case BiomeID::Underworld:
            if (surroundings.depth_above < worldgen_state.r * 4) {
                if (b_pos.y > lava_level - 2 + worldgen_state.r * 4)
                    worldgen_state.block_id = BlockID::MoltenRock;
                else
                    worldgen_state.block_id = BlockID::Bedrock;
            } else if (surroundings.depth_below < worldgen_state.r * 4) {
                worldgen_state.block_id = BlockID::Bedrock;
            }
            break;
        case BiomeID::Plains:
        case BiomeID::Forest:
            if (!surroundings.under_water) {
                if (surroundings.depth_above == 0) {
                    worldgen_state.block_id = BlockID::Grass;
                } else if (surroundings.depth_below < worldgen_state.r * 2) {
                    worldgen_state.block_id = BlockID::Cobblestone;
                } else if (surroundings.depth_above < 4 + worldgen_state.r * 6) {
                    worldgen_state.block_id = BlockID::Dirt;
                } else if (surroundings.depth_above < 15) {
                    if (worldgen_state.r < 0.1)
                        worldgen_state.block_id = BlockID::Dirt;
                    else if (worldgen_state.r < 0.2)
                        worldgen_state.block_id = BlockID::Gravel;
                }
            }
            break;
        case BiomeID::Desert:
            if (surroundings.depth_above < 2 + worldgen_state.r * 2) {
                worldgen_state.block_id = BlockID::Sand;
            } else if (surroundings.depth_above < 4 + worldgen_state.r * 6) {
                worldgen_state.block_id = BlockID::Sandstone;
            } else if (surroundings.depth_above < 15) {
                if (worldgen_state.r < 0.1)
                    worldgen_state.block_id = BlockID::Sand;
                else if (worldgen_state.r < 0.2)
                    worldgen_state.block_id = BlockID::Gravel;
            }
            break;
        }
    } else if (worldgen_state.block_id == BlockID::Air &&
               !surroundings.above_water) {
        switch (worldgen_state.biome_id) {
        case BiomeID::Plains:
            if (surroundings.depth_below == 0) {
                if (worldgen_state.r < 0.10) {
                    worldgen_state.block_id = BlockID::TallGrass;
                } else if (worldgen_state.r < 0.11) {
                    worldgen_state.block_id = BlockID::Leaves;
                }
            }
            break;
        case BiomeID::Forest:
            if (worldgen_state.r_xz < 0.01) {
                int trunk_height = int(5 + worldgen_state.r_xz * 400);
                if (surroundings.depth_below < trunk_height) {
                    worldgen_state.block_id = BlockID::Log;
                }
            } else if (worldgen_state.r < 0.6 && surroundings.depth_below == 0) {
                worldgen_state.block_id = BlockID::Leaves;
            }
            break;
        case BiomeID::Desert:
            if (worldgen_state.r_xz < 0.001) {
                int trunk_height = int(5 + worldgen_state.r_xz * 400);
                if (surroundings.depth_below < trunk_height) {
                    worldgen_state.block_id = BlockID::Cactus;
                }
            } else if (worldgen_state.r < 0.02 && surroundings.depth_below == 0) {
                worldgen_state.block_id = BlockID::DriedShrub;
            }
            break;
        default:
            break;
        }
    }
}

BlockID gen_block(in float3 b_pos) {
    WorldgenState worldgen_state = get_worldgen_state(b_pos);

    biome_pass0(worldgen_state, b_pos);
    block_pass0(worldgen_state, b_pos);
    SurroundingInfo surroundings = get_surrounding(worldgen_state, b_pos);
    block_pass2(worldgen_state, b_pos, surroundings);

    return worldgen_state.block_id;
}

[numthreads(8, 8, 8)] void main(uint3 global_i
                                : SV_DispatchThreadID) {
    float3 block_pos =
        float3(global_i) + p.pos.xyz - float3(BLOCK_NX, BLOCK_NY, BLOCK_NY) / 2;

    uint chunk_texture_id = p.output_image_i;
    RWTexture3D<uint> chunk = daxa::getRWTexture3D<uint>(chunk_texture_id);

    chunk[int3(global_i)] = (uint)gen_block(block_pos);
}
