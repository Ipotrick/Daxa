#include "common.hlsl"
#include "utils/noise.hlsl"

#define CHUNK_SIZE 64

struct Push
{
    float3 chunk_pos;
    daxa::BufferId buffer_id;
};
[[vk::push_constant]] const Push p;

struct WorldgenState
{
    float3 pos;
    float t_noise;
    float r, r_xz;
};

WorldgenState get_worldgen_state(float3 pos)
{
    WorldgenState result;
    pos = pos * 0.5;
    result.pos = pos;
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0,
        /* .persistance = */ 0.12,
        /* .scale       = */ 0.03,
        /* .lacunarity  = */ 4.7,
        /* .octaves     = */ 2,
    };
    float val = fractal_noise(pos + 100.0, noise_conf);
    // val = val - (-pos.y + 30.0) * 0.04;
    // val -= pow(smoothstep(-1.0, 1.0, -pos.y + 32.0), 2.0) * 0.15;
    result.t_noise = val;
    result.r = rand(pos);
    result.r_xz = rand(pos * float3(13.1, 0, 13.1) + 0.17);
    return result;
}

struct Chunk
{
    BlockID voxels[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

    BlockID generate_block_id(in WorldgenState worldgen_state)
    {
        float val = worldgen_state.t_noise;
        val = max(val, 0.0);
        if (val > 0.0)
            return BlockID::Stone;
        if (worldgen_state.pos.y > 33.0)
            return BlockID::Water;
        return BlockID::Air;
    }

    void chunkgen(uint3 block_offset)
    {
        float3 block_pos = float3(block_offset) + p.chunk_pos;
        WorldgenState worldgen_state = get_worldgen_state(block_pos);
        BlockID id = generate_block_id(worldgen_state);

        if (id == BlockID::Stone)
        {
            uint above_i;
            for (above_i = 0; above_i < 6; ++above_i)
            {
                WorldgenState temp_worldgen_state = get_worldgen_state(block_pos - float3(0, above_i + 1, 0));
                if (generate_block_id(temp_worldgen_state) == BlockID::Air)
                    break;
            }
            uint r = (uint)(worldgen_state.r * 10);
            switch (r)
            {
            case 0: id = BlockID::Gravel; break;
            case 1: id = BlockID::Cobblestone; break;
            default: break;
            }
            if (above_i == 0)
                id = BlockID::Grass;
            else if (above_i < 4)
                id = BlockID::Dirt;
        }
        else if (id == BlockID::Air)
        {
            uint below_i;
            for (below_i = 0; below_i < 6; ++below_i)
            {
                WorldgenState temp_worldgen_state = get_worldgen_state(block_pos + float3(0, below_i + 1, 0));
                if (generate_block_id(temp_worldgen_state) == BlockID::Stone)
                    break;
            }
            if (below_i == 0)
            {
                uint r = (uint)(worldgen_state.r * 100);
                if (r < 50)
                {
                    switch (r)
                    {
                    case 0: id = BlockID::Rose; break;
                    default:
                        id = BlockID::TallGrass;
                        break;
                    }
                }
            }
        }

        uint voxel_index = block_offset.x + block_offset.y * CHUNK_SIZE + block_offset.z * CHUNK_SIZE * CHUNK_SIZE;
        voxels[voxel_index] = id;
    }
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(Chunk)

// clang-format off
[numthreads(8, 8, 8)] void main(uint3 block_offset : SV_DispatchThreadID)
// clang-format on
{
    StructuredBuffer<Chunk> chunk = daxa::get_StructuredBuffer<Chunk>(p.buffer_id);
    chunk[0].chunkgen(block_offset);
}
