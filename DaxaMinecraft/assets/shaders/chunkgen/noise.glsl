#include "utils/noise.glsl"

float terrain_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 0.1f,
        /* .persistance = */ 0.6f,
        /* .scale       = */ 0.01f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 4,
    };
    return fractal_noise(pos, noise_conf) * 3.0f + pos.y * 0.01 - 0.3;
}

float biome_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 0.1f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.005f,
        /* .lacunarity  = */ 4,
        /* .octaves     = */ 4,
    };
    return fractal_noise(pos + 1000, noise_conf) * 3.0f;
}
