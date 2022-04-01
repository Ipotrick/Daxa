#include "utils/noise.glsl"

float terrain_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 0.1f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.002f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 6,
    };
    return fractal_noise(pos, noise_conf) * 10.0f + pos.y * 0.015 - 1.6;
}

float biome_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 0.1f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.001f,
        /* .lacunarity  = */ 4,
        /* .octaves     = */ 4,
    };
    return fractal_noise(vec3(pos.x, 0, pos.z) + 1000, noise_conf) * 3.0f;
}

float underworld_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 0.1f,
        /* .persistance = */ 0.5f,
        /* .scale       = */ 0.002f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 6,
    };
    return fractal_noise(pos - vec3(0, 0, -1000), noise_conf) * 10.0f + abs(500 - pos.y) * 0.006 - 0.2;
}
