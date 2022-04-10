#pragma once

#include "utils/noise.hlsl"
#include "block_info.hlsl"

float terrain_noise(float3 pos) {
    FractalNoiseConfig noise_conf1 = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.008f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 3,
    };
    FractalNoiseConfig noise_conf2 = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.5f,
        /* .scale       = */ 0.01f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 5,
    };
    float val1 = fractal_noise(pos, noise_conf1);
    return val1 - 0.5;
    // float val2 = fractal_noise(float3(pos.x, 0, pos.z), noise_conf2);
    // float riv = (abs(val2 - 0.7)) * 2;
    // return ((pos.y - 65) * 0.2 + riv + (val1 - 0.5) * 6.4) * riv - 0.5;
}

float biome_noise(float3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.001f,
        /* .lacunarity  = */ 4,
        /* .octaves     = */ 4,
    };
    return fractal_noise(float3(pos.x, 0, pos.z) + 1000, noise_conf);
}

float underworld_noise(float3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.5f,
        /* .scale       = */ 0.002f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 6,
    };
    return fractal_noise(pos, noise_conf) + abs(400 - pos.y) * 0.015 - 1.0;
}

float cave_noise(float3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.02f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 4,
    };
    return fractal_noise(pos, noise_conf) + abs(220 - pos.y) * 0.01 - 0.5;
}

struct WorldgenState {
    float t_noise;
    float b_noise;
    float u_noise;
    float c_noise;

    float r;
    float r_xz;

    BlockID block_id;
    BiomeID biome_id;
};

WorldgenState get_worldgen_state(float3 pos) {
    WorldgenState result;
    result.t_noise = terrain_noise(pos);
    result.b_noise = biome_noise(pos);
    result.u_noise = underworld_noise(pos);
    result.c_noise = cave_noise(pos);
    result.r = rand(pos);
    result.r_xz = rand(float3(pos.x, 0, pos.z));
    return result;
}
