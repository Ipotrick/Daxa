#pragma once
#include <cstdint>
#include <glm/glm.hpp>

using uint = std::uint32_t;

const uint perm[64] = {
    // clang-format off
    0x5b89a097, 0x0d830f5a, 0x35605fc9, 0xe107e9c2, 0x1e67248c, 0x63088e45, 0x0a15f025, 0x9406be17,
    0x4bea78f7, 0x3ec51a00, 0xcbdbfc5e, 0x200b2375, 0x5821b139, 0x573895ed, 0x887d14ae, 0xaf44a8ab,
    0x8647a54a, 0xa61b308b, 0xe79e924d, 0x7ae56f53, 0xe685d33c, 0x295c69dc, 0x28f52e37, 0x368f66f4,
    0xa13f1941, 0x4950d801, 0xbb844cd1, 0xa91259d0, 0x8287c4c8, 0x569fbc74, 0xc66d64a4, 0x4003baad,
    0xfae2d934, 0xca057b7c, 0x7e769326, 0xd45552ff, 0xe33bcecf, 0x113a102f, 0x2a1cbdb6, 0xd5aab7df,
    0x0298f877, 0x46a39a2c, 0x9b6599dd, 0x09ac2ba7, 0xfd271681, 0x6e6c6213, 0xe8e0714f, 0x6870b9b2,
    0xe461f6da, 0xc1f222fb, 0x0c90d2ee, 0xf1a2b3bf, 0xeb913351, 0x6bef0ef9, 0x1fd6c031, 0x9d6ac7b5,
    0xb0cc54b8, 0x2d327973, 0xfe96047f, 0x5dcdec8a, 0x1d4372de, 0x8df34818, 0x424ec380, 0xb49c3dd7,
    // clang-format on
};

uint hash(uint i) {
    uint shift = i & 0x3;
    return (perm[(i >> 2) & 0x3f] >> shift) & 0xff;
}

float grad(uint hashValue, float x) {
    const uint h = hashValue & 0x0F; // Convert low 4 bits of hash code
    float grad = 1.0f + (h & 7);     // Gradient value 1.0, 2.0, ..., 8.0
    if ((h & 8) != 0)
        grad = -grad;  // Set a random sign for the gradient
    return (grad * x); // Multiply the gradient with the distance
}

float grad(uint hashValue, float x, float y) {
    const uint h = hashValue & 0x3F; // Convert low 3 bits of hash code
    const float u = h < 4 ? x : y;   // into 8 simple gradient directions,
    const float v = h < 4 ? y : x;
    return ((h & 1) != 0 ? -u : u) +
           ((h & 2) != 0 ? -2.0f * v
                         : 2.0f * v); // and compute the dot product with (x,y).
}

float grad(uint hashValue, float x, float y, float z) {
    uint h = hashValue & 15; // Convert low 4 bits of hash code into 12 simple
    float u = h < 8 ? x : y; // gradient directions, and compute dot product.
    float v = h < 4 ? y : h == 12 || h == 14 ? x
                                             : z; // Fix repeats at h = 12 to 15
    return ((h & 1) != 0 ? -u : u) + ((h & 2) != 0 ? -v : v);
}

uint fastfloor(float fp) {
    uint i = uint(fp);
    return (fp < i) ? (i - 1) : (i);
}

float noise(float x, float y, float z) {
    float n0, n1, n2, n3; // Noise contributions from the four corners

    // Skewing/Unskewing factors for 3D
    const float F3 = 1.0f / 3.0f;
    const float G3 = 1.0f / 6.0f;

    // Skew the input space to determine which simplex cell we're in
    float s = (x + y + z) * F3; // Very nice and simple skew factor for 3D
    uint i = fastfloor(x + s);
    uint j = fastfloor(y + s);
    uint k = fastfloor(z + s);
    float t = (i + j + k) * G3;
    float X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; // The x,y,z distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0; // X Y Z order
        } else if (x0 >= z0) {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 0;
            k2 = 1; // X Z Y order
        } else {
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 1;
            j2 = 0;
            k2 = 1; // Z X Y order
        }
    } else { // x0<y0
        if (y0 < z0) {
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 0;
            j2 = 1;
            k2 = 1; // Z Y X order
        } else if (x0 < z0) {
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 0;
            j2 = 1;
            k2 = 1; // Y Z X order
        } else {
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0; // Y X Z order
        }
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f * G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f * G3;
    float z2 = z0 - k2 + 2.0f * G3;
    float x3 = x0 - 1.0f + 3.0f * G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0f + 3.0f * G3;
    float z3 = z0 - 1.0f + 3.0f * G3;

    // Work out the hashed gradient indices of the four simplex corners
    uint gi0 = hash(i + hash(j + hash(k)));
    uint gi1 = hash(i + i1 + hash(j + j1 + hash(k + k1)));
    uint gi2 = hash(i + i2 + hash(j + j2 + hash(k + k2)));
    uint gi3 = hash(i + 1 + hash(j + 1 + hash(k + 1)));

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0) {
        n0 = 0.0;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * grad(gi0, x0, y0, z0);
    }
    float t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0) {
        n1 = 0.0;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * grad(gi1, x1, y1, z1);
    }
    float t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0) {
        n2 = 0.0;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * grad(gi2, x2, y2, z2);
    }
    float t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0) {
        n3 = 0.0;
    } else {
        t3 *= t3;
        n3 = t3 * t3 * grad(gi3, x3, y3, z3);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0f * (n0 + n1 + n2 + n3);
}

struct FractalNoiseConfig {
    float amplitude;
    float persistance;
    float scale;
    float lacunarity;
    uint octaves;
};

float fractalNoise(glm::vec3 pos, FractalNoiseConfig config) {
    float value = 0.0f;
    for (int i = 0; i < config.octaves; ++i) {
        value += noise(pos.x * config.scale, pos.y * config.scale, pos.z * config.scale) *
                 config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value;
}
