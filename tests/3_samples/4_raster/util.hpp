#pragma once

#include <glm/glm.hpp>

u32 asuint(f32 x)
{
    return *reinterpret_cast<u32 *>(&x);
}
u32 asuint(glm::vec2 v)
{
    f32 a = v.x;
    f32 b = v.y + static_cast<f32>(asuint(a));
    return asuint(b);
}
u32 asuint(glm::vec3 v)
{
    f32 a = v.x;
    f32 b = v.y + static_cast<f32>(asuint(a));
    f32 c = v.z + static_cast<f32>(asuint(b));
    return asuint(c);
}
u32 asuint(glm::vec4 v)
{
    f32 a = v.x;
    f32 b = v.y + static_cast<f32>(asuint(a));
    f32 c = v.z + static_cast<f32>(asuint(b));
    f32 d = v.w + static_cast<f32>(asuint(c));
    return asuint(d);
}
f32 asfloat(u32 x)
{
    return *reinterpret_cast<f32 *>(&x);
}

f32 frac(f32 x)
{
    return x - static_cast<f32>(static_cast<i32>(x));
}
glm::vec3 frac(glm::vec3 v)
{
    return glm::vec3(frac(v.x), frac(v.y), frac(v.z));
}
f32 mix(f32 a, f32 b, f32 f)
{
    return a * (1.0f - f) + b * f;
}
f32 smoothstep(f32 lo, f32 hi, f32 x)
{
    if (x < lo)
        return 0.0f;
    if (x >= hi)
        return 1.0f;

    x = (x - lo) / (hi - lo);
    return x * x * (3.0f - 2.0f * x);
}

u32 rand_hash(u32 x)
{
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}
u32 rand_hash(glm::uvec2 v) { return rand_hash(v.x ^ rand_hash(v.y)); }
u32 rand_hash(glm::uvec3 v)
{
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z));
}
u32 rand_hash(glm::uvec4 v)
{
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z) ^ rand_hash(v.w));
}
f32 rand_float_construct(u32 m)
{
    const u32 ieee_mantissa = 0x007FFFFFu;
    const u32 ieee_one = 0x3F800000u;
    m &= ieee_mantissa;
    m |= ieee_one;
    float f = asfloat(m);
    return f - 1.0f;
}
f32 rand(f32 x) { return rand_float_construct(rand_hash(asuint(x))); }
f32 rand(glm::vec2 v) { return rand_float_construct(rand_hash(asuint(v))); }
f32 rand(glm::vec3 v) { return rand_float_construct(rand_hash(asuint(v))); }
f32 rand(glm::vec4 v) { return rand_float_construct(rand_hash(asuint(v))); }
glm::vec2 rand_vec2(glm::vec2 v) { return glm::vec2(rand(v.x), rand(v.y)); }
glm::vec3 rand_vec3(glm::vec3 v) { return glm::vec3(rand(v.x), rand(v.y), rand(v.z)); }
glm::vec4 rand_vec4(glm::vec4 v)
{
    return glm::vec4(rand(v.x), rand(v.y), rand(v.z), rand(v.w));
}
f32 noise(glm::vec3 x)
{
    const glm::vec3 st = glm::vec3(110, 241, 171);
    glm::vec3 i = floor(x);
    glm::vec3 f = frac(x);
    f32 n = glm::dot(i, st);
    glm::vec3 u = f * f * (3.0f - 2.0f * f);
    f32 r = mix(
        mix(
            mix(rand(n + glm::dot(st, glm::vec3(0, 0, 0))),
                 rand(n + glm::dot(st, glm::vec3(1, 0, 0))), u.x),
            mix(rand(n + glm::dot(st, glm::vec3(0, 1, 0))),
                 rand(n + glm::dot(st, glm::vec3(1, 1, 0))), u.x),
            u.y),
        mix(
            mix(rand(n + glm::dot(st, glm::vec3(0, 0, 1))),
                 rand(n + glm::dot(st, glm::vec3(1, 0, 1))), u.x),
            mix(rand(n + glm::dot(st, glm::vec3(0, 1, 1))),
                 rand(n + glm::dot(st, glm::vec3(1, 1, 1))), u.x),
            u.y),
        u.z);
    return r * 2.0f - 1.0f;
}

struct FractalNoiseConfig
{
    f32 amplitude;
    f32 persistance;
    f32 scale;
    f32 lacunarity;
    u32 octaves;
};

float fractal_noise(glm::vec3 pos, FractalNoiseConfig config)
{
    float value = 0.0f;
    float max_value = 0.0f;
    float amplitude = config.amplitude;
    glm::mat3 rot_mat = glm::mat3(
        0.2184223f, -0.5347182f, 0.8163137f,
        0.9079879f, -0.1951438f, -0.3707788f,
        0.3575608f, 0.8221893f, 0.4428939f);
    for (u32 i = 0; i < config.octaves; ++i)
    {
        pos = (pos * rot_mat) + glm::vec3(71.444f, 25.170f, -54.766f);
        glm::vec3 p = pos * config.scale;
        value += noise(p) * config.amplitude;
        max_value += config.amplitude;
        config.amplitude *= config.persistance;
        config.scale *= config.lacunarity;
    }
    return value / max_value * amplitude;
}