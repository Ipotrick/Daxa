#pragma once

#include "utils/noise.hlsl"

struct WorldgenState
{
    f32vec3 pos;
    f32 t_noise;
    f32 r, r_xz;
};

WorldgenState get_worldgen_state(f32vec3 pos)
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
    f32 val = fractal_noise(pos + 100.0, noise_conf);
    // val = val - (-pos.y + 30.0) * 0.04;
    // val -= pow(smoothstep(-1.0, 1.0, -pos.y + 32.0), 2.0) * 0.15;
    result.t_noise = val;
    result.r = rand(pos);
    result.r_xz = rand(pos * f32vec3(13.1, 0, 13.1) + 0.17);
    return result;
}
