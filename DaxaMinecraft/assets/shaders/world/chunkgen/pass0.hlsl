#include "world/common.hlsl"
#include "world/chunkgen/noise.hlsl"

#include "utils/shape_dist.hlsl"

#include "core.hlsl"

#define water_level 164
#define lava_level 458

void biome_pass0(in out WorldgenState worldgen_state, in float3 b_pos) {
    worldgen_state.biome_id = BiomeID::Plains;
    if (worldgen_state.b_noise < 1.2) {
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
    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globals_sb);
    uint3 chunk_i = p.pos.xyz / CHUNK_SIZE;

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
            // if (surroundings.depth_below == 0) {
            //     if (worldgen_state.r < 0.10) {
            //         worldgen_state.block_id = BlockID::TallGrass;
            //     } else if (worldgen_state.r < 0.11) {
            //         worldgen_state.block_id = BlockID::Leaves;
            //     }
            // }
            break;
        case BiomeID::Forest:
            if (worldgen_state.r_xz < 0.01) {
                int trunk_height = 1; // int(5 + worldgen_state.r_xz * 400);
                if (surroundings.depth_below < trunk_height) {
                    if (globals[0].chunkgen_data[chunk_i.z][chunk_i.y][chunk_i.x].structure_n < 127) {
                        int structure_n;
                        InterlockedAdd(globals[0].chunkgen_data[chunk_i.z][chunk_i.y][chunk_i.x].structure_n, 1, structure_n);
                        globals[0].chunkgen_data[chunk_i.z][chunk_i.y][chunk_i.x].structures[structure_n].p = float4(b_pos, 0);
                        globals[0].chunkgen_data[chunk_i.z][chunk_i.y][chunk_i.x].structures[structure_n].id = (int(b_pos.x + 10000) % 5 == 0) ? 2 : 1;
                    }
                    // worldgen_state.block_id = BlockID::Log;
                }
            } else if (worldgen_state.r < 0.6 && surroundings.depth_below == 0) {
                worldgen_state.block_id = BlockID::TallGrass;
            }
            break;
        case BiomeID::Desert:
            // if (worldgen_state.r_xz < 0.001) {
            //     int trunk_height = int(5 + worldgen_state.r_xz * 400);
            //     if (surroundings.depth_below < trunk_height) {
            //         worldgen_state.block_id = BlockID::Cactus;
            //     }
            // } else if (worldgen_state.r < 0.02 && surroundings.depth_below == 0) {
            //     worldgen_state.block_id = BlockID::DriedShrub;
            // }
            break;
        default:
            break;
        }
    }
}

void floor_pass(in out WorldgenState worldgen_state, float3 b_pos) {
    worldgen_state.block_id = BlockID::Air;

    if (b_pos.y == 1)
        worldgen_state.block_id = BlockID::Grass;
}

void shapes_pass(in out WorldgenState worldgen_state, float3 b_pos) {
    if (sd_sphere(b_pos, float3(20, 10, 20), 8) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_capsule(b_pos, float3(60, 10, 20), float3(60, 40, 20), 8) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_box(b_pos, float3(100, 10, 20), float3(8, 8, 8)) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_box_frame(b_pos, float3(140, 10, 20), float3(8, 8, 8), 2) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_ellipsoid(b_pos, float3(180, 20, 20), float3(8, 18, 6)) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_torus(b_pos, float3(220, 6, 20), float2(10, 4)) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_tri_prism(b_pos, float3(260, 12, 20), float2(20, 4)) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_cone(b_pos, float3(300, 10, 20), float3(290, 20, 50), 8, 2) < 0)
        worldgen_state.block_id = BlockID::Stone;

    if (sd_cylinder(b_pos, float3(340, 5, 20), float3(330, 40, 30), 8) < 0)
        worldgen_state.block_id = BlockID::Stone;
}

void tree_pass0(in out WorldgenState worldgen_state, float3 b_pos) {
    float3 tree_pos = float3(50, 50, 50);
    float3 trunk_point = tree_pos;
    float3 branch_points[45];
    int branch_n = 0;
    float scl = 0.3f;
    for (int i = 0; i < 45; ++i) {
        if (rand(3.74 * i) > 2.0 - 0.1 * i) {
            branch_points[branch_n] = trunk_point;
            ++branch_n;
        }
        float3 next_point = trunk_point + float3((noise(12.6 + 0.24 * i) - 0.5) * 3, 3, (noise(0.2 + 0.51 * i) - 0.5) * 3) * scl;
        if (sd_capsule(b_pos, trunk_point, next_point, (6 - 0.1 * i) * scl) < 0) //
            worldgen_state.block_id = BlockID::Log;
        trunk_point = next_point;
    }
    for (int j = 0; j < branch_n; ++j) {
        for (int i = 0; i < 4; ++i) {
            float v = 50 / (i + 1);
            float3 next_point = branch_points[j] + float3((noise(120.6 + 100 * (i + j * 17)) - 0.5) * v, 10, (noise(10.2 + 100 * (i + j * 42)) - 0.5) * v) * scl;
            if (sd_capsule(b_pos, branch_points[j], next_point, (2.5 - 0.5 * i) * scl) < 0) // 4.5 - 0.6 * i
                worldgen_state.block_id = BlockID::Log;
            branch_points[j] = next_point;

            float leaves_sphere_dist;
            leaves_sphere_dist = sd_sphere(b_pos, next_point, 25 * scl);
            if (leaves_sphere_dist < 0 && worldgen_state.r > 0.95 - leaves_sphere_dist * leaves_sphere_dist * 0.001) {
                worldgen_state.block_id = BlockID::Leaves;
            }
        }
    }
}

void tree_pass(in out WorldgenState worldgen_state, float3 b_pos) {
    float3 center = float3(100, 0, 100);

    float3 pine_trees[] = {
        float3(32, 1, 18) + center,
        float3(41, 1, 0) + center,
        float3(55, 1, 12) + center,
        float3(69, 1, 3) + center,
    };

    float3 spruce_trees[] = {
        float3(20, 2, 20) + center,
        float3(40, 2, 25) + center,
        float3(30, 2, 30) + center,
        float3(24, 2, 10) + center,
        float3(38, 2, 10) + center,
        float3(45, 2, 17) + center,
        float3(60, 2, 10) + center,
    };

    for (int i = 0; i < sizeof(pine_trees) / sizeof(pine_trees[0]); ++i) {
        TreeSDF pine0 = sd_tree_pine(b_pos, worldgen_state.r, pine_trees[i]);

        if (pine0.trunk_dist < 0)
            worldgen_state.block_id = BlockID::Log;
        if (pine0.leaves_dist < 0)
            worldgen_state.block_id = BlockID::Leaves;
    }

    for (int i = 0; i < sizeof(spruce_trees) / sizeof(spruce_trees[0]); ++i) {
        TreeSDF spruce0 = sd_tree_spruce(b_pos, worldgen_state.r, spruce_trees[i]);

        if (spruce0.trunk_dist < 0)
            worldgen_state.block_id = BlockID::Log;
        if (spruce0.leaves_dist < 0)
            worldgen_state.block_id = BlockID::Leaves;
    }

    // if (b_pos.z == tree_pos.z)
    //     if (fmod(trunk_dist, 4) < 1)
    //         worldgen_state.block_id = BlockID::Sand;

    // if (b_pos.z == tree_pos.z)
    //     if (fmod(leaves_dist, 4) < 1.2)
    //         worldgen_state.block_id = BlockID::Brick;
}

BlockID gen_block(in float3 b_pos) {
    WorldgenState worldgen_state = get_worldgen_state(b_pos);

    // floor_pass(worldgen_state, b_pos);
    // shapes_pass(worldgen_state, b_pos);
    // tree_pass(worldgen_state, b_pos);

    biome_pass0(worldgen_state, b_pos);
    block_pass0(worldgen_state, b_pos);
    SurroundingInfo surroundings = get_surrounding(worldgen_state, b_pos);
    block_pass2(worldgen_state, b_pos, surroundings);

    return worldgen_state.block_id;
}

[numthreads(8, 8, 8)] void main(uint3 global_i
                                : SV_DispatchThreadID) {
    float3 block_pos = float3(global_i) + p.pos.xyz; // - float3(BLOCK_NX, BLOCK_NY, BLOCK_NY) / 2;
    // block_pos.y = BLOCK_NY - block_pos.y;

    uint chunk_texture_id = p.output_image_i;
    RWTexture3D<uint> chunk = daxa::getRWTexture3D<uint>(chunk_texture_id);

    chunk[int3(global_i)] = (uint)gen_block(block_pos);
}
