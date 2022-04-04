
#include <utils/noise.glsl>

float terrain_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.01f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 4,
    };
    return fractal_noise(pos, noise_conf);
}

float biome_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 0.3f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.001f,
        /* .lacunarity  = */ 4,
        /* .octaves     = */ 4,
    };
    return fractal_noise(vec3(pos.x, 0, pos.z) + 1000, noise_conf);
}

float underworld_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.5f,
        /* .scale       = */ 0.002f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 6,
    };
    return fractal_noise(pos, noise_conf) + abs(340 - pos.y) * 0.015 - 1.0;
}

float cave_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.02f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 4,
    };
    return fractal_noise(pos, noise_conf) + abs(200 - pos.y) * 0.02 - 0.5;
}
