#pragma once

#include "utils/rand.hlsl"

float noise(float3 x)
{
    const float3 st = float3(110, 241, 171);
    float3 i = floor(x);
    float3 f = frac(x);
    float n = dot(i, st);
    float3 u = f * f * (3.0 - 2.0 * f);
    float r = lerp(
        lerp(
            lerp(rand(n + dot(st, float3(0, 0, 0))),
                 rand(n + dot(st, float3(1, 0, 0))), u.x),
            lerp(rand(n + dot(st, float3(0, 1, 0))),
                 rand(n + dot(st, float3(1, 1, 0))), u.x),
            u.y),
        lerp(
            lerp(rand(n + dot(st, float3(0, 0, 1))),
                 rand(n + dot(st, float3(1, 0, 1))), u.x),
            lerp(rand(n + dot(st, float3(0, 1, 1))),
                 rand(n + dot(st, float3(1, 1, 1))), u.x),
            u.y),
        u.z);
    return r * 2.0 - 1.0;
}

struct FractalNoiseConfig
{
    float amplitude;
    float persistance;
    float scale;
    float lacunarity;
    uint octaves;
};

float fractal_noise(float3 pos, FractalNoiseConfig config)
{
    float value = 0.0;
    float max_value = 0.0;
    float amplitude = config.amplitude;
    float3x3 rot_mat = float3x3(
        0.2184223, -0.5347182, 0.8163137,
        0.9079879, -0.1951438, -0.3707788,
        0.3575608, 0.8221893, 0.4428939);
    for (uint i = 0; i < config.octaves; ++i)
    {
        pos = mul(rot_mat, pos) + float3(71.444, 25.170, -54.766);
        float3 p = pos * config.scale;
        value += noise(p) * config.amplitude;
        max_value += config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value / max_value * amplitude;
}

float voronoi_noise(float3 pos)
{
    float value = 1e38;

    for (int zi = 0; zi < 3; ++zi)
    {
        for (int yi = 0; yi < 3; ++yi)
        {
            for (int xi = 0; xi < 3; ++xi)
            {
                float3 p = pos + float3(xi - 1, yi - 1, zi - 1);
                p = floor(p) + 0.5;
                p += float3(rand(p + 71.444), rand(p + 25.170), rand(p + -54.766));
                value = min(value, dot(pos - p, pos - p));
            }
        }
    }

    return value;
}

float fractal_voronoi_noise(float3 pos, FractalNoiseConfig config)
{
    float value = 0.0;
    float max_value = 0.0;
    float amplitude = config.amplitude;
    float3x3 rot_mat = float3x3(
        0.2184223, -0.5347182, 0.8163137,
        0.9079879, -0.1951438, -0.3707788,
        0.3575608, 0.8221893, 0.4428939);
    for (uint i = 0; i < config.octaves; ++i)
    {
        pos = mul(rot_mat, pos) + float3(71.444, 25.170, -54.766);
        float3 p = pos * config.scale;
        value += voronoi_noise(p) * config.amplitude;
        max_value += config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value / max_value * amplitude;
}
