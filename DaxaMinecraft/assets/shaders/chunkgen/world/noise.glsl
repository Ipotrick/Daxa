
#include <utils/noise.glsl>

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
    return fractal_noise(pos, noise_conf) * 10.0f + abs(600 - pos.y) * 0.015 - 1.0;
}

float cave_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 0.1f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.02f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 4,
    };
    return fractal_noise(pos, noise_conf) * 10.0f + abs(260 - pos.y) * 0.02 - 0.5;
}
