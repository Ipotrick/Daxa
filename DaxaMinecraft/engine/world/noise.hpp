#pragma once
#include <cstdint>
#include <glm/glm.hpp>
using uint = std::uint32_t;

const uint perm[64] = {
    0x5b89a097, 0x0d830f5a, 0x35605fc9, 0xe107e9c2, 0x1e67248c, 0x63088e45, 0x0a15f025, 0x9406be17,
    0x4bea78f7, 0x3ec51a00, 0xcbdbfc5e, 0x200b2375, 0x5821b139, 0x573895ed, 0x887d14ae, 0xaf44a8ab,
    0x8647a54a, 0xa61b308b, 0xe79e924d, 0x7ae56f53, 0xe685d33c, 0x295c69dc, 0x28f52e37, 0x368f66f4,
    0xa13f1941, 0x4950d801, 0xbb844cd1, 0xa91259d0, 0x8287c4c8, 0x569fbc74, 0xc66d64a4, 0x4003baad,
    0xfae2d934, 0xca057b7c, 0x7e769326, 0xd45552ff, 0xe33bcecf, 0x113a102f, 0x2a1cbdb6, 0xd5aab7df,
    0x0298f877, 0x46a39a2c, 0x9b6599dd, 0x09ac2ba7, 0xfd271681, 0x6e6c6213, 0xe8e0714f, 0x6870b9b2,
    0xe461f6da, 0xc1f222fb, 0x0c90d2ee, 0xf1a2b3bf, 0xeb913351, 0x6bef0ef9, 0x1fd6c031, 0x9d6ac7b5,
    0xb0cc54b8, 0x2d327973, 0xfe96047f, 0x5dcdec8a, 0x1d4372de, 0x8df34818, 0x424ec380, 0xb49c3dd7,
};

struct FractalNoiseConfig {
    float amplitude;
    float persistance;
    float scale;
    float lacunarity;
    uint  octaves;
};

float noise(glm::vec3 pos) {
    return -1;
}

float fractal_noise(glm::vec3 pos, FractalNoiseConfig config) {
    float value = 0.0f;
    for (int i = 0; i < config.octaves; ++i) {
        value += noise(pos * config.scale) * config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value;
}
