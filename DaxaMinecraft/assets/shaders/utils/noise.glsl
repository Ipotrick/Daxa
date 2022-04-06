
#include <utils/rand.glsl>

float noise(vec3 x) {
    const vec3 st = vec3(110, 241, 171);
    vec3 i = floor(x);
    vec3 f = fract(x);
    float n = dot(i, st);
    vec3 u = f * f * (3.0 - 2.0 * f);
    float r = mix(mix(mix(rand(n + dot(st, vec3(0, 0, 0))), rand(n + dot(st, vec3(1, 0, 0))), u.x),
                      mix(rand(n + dot(st, vec3(0, 1, 0))), rand(n + dot(st, vec3(1, 1, 0))), u.x), u.y),
                  mix(mix(rand(n + dot(st, vec3(0, 0, 1))), rand(n + dot(st, vec3(1, 0, 1))), u.x),
                      mix(rand(n + dot(st, vec3(0, 1, 1))), rand(n + dot(st, vec3(1, 1, 1))), u.x), u.y),
                  u.z);
    return r * 2 - 0.5;
}

struct FractalNoiseConfig {
    float amplitude;
    float persistance;
    float scale;
    float lacunarity;
    uint octaves;
};

float fractal_noise(vec3 pos, FractalNoiseConfig config) {
    float value = 0.0;
    for (int i = 0; i < config.octaves; ++i) {
        vec3 p = pos * config.scale;
        value += noise(p) * config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value;
}
