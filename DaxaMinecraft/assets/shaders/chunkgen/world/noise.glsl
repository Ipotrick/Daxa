#include <utils/noise.glsl>

float terrain_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
        /* .persistance = */ 0.4f,
        /* .scale       = */ 0.01f,
        /* .lacunarity  = */ 2,
        /* .octaves     = */ 6,
    };
    return fractal_noise(pos, noise_conf) - 1.5; // + (pos.y - 120) * 0.015;
}

float biome_noise(vec3 pos) {
    FractalNoiseConfig noise_conf = {
        /* .amplitude   = */ 1.0f,
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
    return fractal_noise(pos, noise_conf) + abs(400 - pos.y) * 0.015 - 1.0;
}

float cave_noise(vec3 pos) {
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

    uint block_id, biome_id;
};

WorldgenState get_worldgen_state(vec3 pos) {
    WorldgenState result;
    result.t_noise = terrain_noise(pos);
    result.b_noise = biome_noise(pos);
    result.u_noise = underworld_noise(pos);
    result.c_noise = cave_noise(pos);
    result.r = rand(pos);
    result.r_xz = rand(vec3(pos.x, 0, pos.z));
    return result;
}
