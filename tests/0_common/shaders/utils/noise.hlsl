#pragma once

#include "utils/rand.hlsl"

f32 noise(f32vec3 x)
{
    const f32vec3 st = f32vec3(110, 241, 171);
    f32vec3 i = floor(x);
    f32vec3 f = frac(x);
    f32 n = dot(i, st);
    f32vec3 u = f * f * (3.0 - 2.0 * f);
    f32 r = lerp(
        lerp(
            lerp(rand(n + dot(st, f32vec3(0, 0, 0))),
                 rand(n + dot(st, f32vec3(1, 0, 0))), u.x),
            lerp(rand(n + dot(st, f32vec3(0, 1, 0))),
                 rand(n + dot(st, f32vec3(1, 1, 0))), u.x),
            u.y),
        lerp(
            lerp(rand(n + dot(st, f32vec3(0, 0, 1))),
                 rand(n + dot(st, f32vec3(1, 0, 1))), u.x),
            lerp(rand(n + dot(st, f32vec3(0, 1, 1))),
                 rand(n + dot(st, f32vec3(1, 1, 1))), u.x),
            u.y),
        u.z);
    return r * 2.0 - 1.0;
}

struct FractalNoiseConfig
{
    f32 amplitude;
    f32 persistance;
    f32 scale;
    f32 lacunarity;
    u32 octaves;
};

f32 fractal_noise(f32vec3 pos, FractalNoiseConfig config)
{
    f32 value = 0.0;
    f32 max_value = 0.0;
    f32 amplitude = config.amplitude;
    f32mat3x3 rot_mat = f32mat3x3(
        0.2184223, -0.5347182, 0.8163137,
        0.9079879, -0.1951438, -0.3707788,
        0.3575608, 0.8221893, 0.4428939);
    for (u32 i = 0; i < config.octaves; ++i)
    {
        pos = mul(rot_mat, pos) + f32vec3(71.444, 25.170, -54.766);
        f32vec3 p = pos * config.scale;
        value += noise(p) * config.amplitude;
        max_value += config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value / max_value * amplitude;
}

f32 voronoi_noise(f32vec3 pos)
{
    f32 value = 1e38;

    for (i32 zi = 0; zi < 3; ++zi)
    {
        for (i32 yi = 0; yi < 3; ++yi)
        {
            for (i32 xi = 0; xi < 3; ++xi)
            {
                f32vec3 p = pos + f32vec3(xi - 1, yi - 1, zi - 1);
                p = floor(p) + 0.5;
                p += f32vec3(rand(p + 71.444), rand(p + 25.170), rand(p + -54.766));
                value = min(value, dot(pos - p, pos - p));
            }
        }
    }

    return value;
}

f32 fractal_voronoi_noise(f32vec3 pos, FractalNoiseConfig config)
{
    f32 value = 0.0;
    f32 max_value = 0.0;
    f32 amplitude = config.amplitude;
    f32mat3x3 rot_mat = f32mat3x3(
        0.2184223, -0.5347182, 0.8163137,
        0.9079879, -0.1951438, -0.3707788,
        0.3575608, 0.8221893, 0.4428939);
    for (u32 i = 0; i < config.octaves; ++i)
    {
        pos = mul(rot_mat, pos) + f32vec3(71.444, 25.170, -54.766);
        f32vec3 p = pos * config.scale;
        value += voronoi_noise(p) * config.amplitude;
        max_value += config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value / max_value * amplitude;
}
