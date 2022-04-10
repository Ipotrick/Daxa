#pragma once

#include "utils/rand.hlsl"

float noise(float3 x) {
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
    return r * 2.0 - 0.5;
}

struct FractalNoiseConfig {
    float amplitude;
    float persistance;
    float scale;
    float lacunarity;
    uint octaves;
};

float fractal_noise(float3 pos, FractalNoiseConfig config) {
    float value = 0.0;
    for (uint i = 0; i < config.octaves; ++i) {
        float3 p = pos * config.scale;
        value += noise(p) * config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value;
}
