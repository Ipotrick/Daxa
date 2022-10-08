#pragma once

#include <utils/math.glsl>

u32 rand_hash(u32 x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}
u32 rand_hash(u32vec2 v) { return rand_hash(v.x ^ rand_hash(v.y)); }
u32 rand_hash(u32vec3 v) {
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z));
}
u32 rand_hash(u32vec4 v) {
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z) ^ rand_hash(v.w));
}
f32 rand_float_construct(u32 m) {
    const u32 ieee_mantissa = 0x007FFFFFu;
    const u32 ieee_one = 0x3F800000u;
    m &= ieee_mantissa;
    m |= ieee_one;
    f32 f = uintBitsToFloat(m);
    return f - 1.0;
}
f32 rand(f32 x) { return rand_float_construct(rand_hash(floatBitsToUint(x))); }
f32 rand(f32vec2 v) { return rand_float_construct(rand_hash(floatBitsToUint(v))); }
f32 rand(f32vec3 v) { return rand_float_construct(rand_hash(floatBitsToUint(v))); }
f32 rand(f32vec4 v) { return rand_float_construct(rand_hash(floatBitsToUint(v))); }

f32vec3 ortho(f32vec3 v) {
    return mix(f32vec3(-v.y, v.x, 0.0), f32vec3(0.0, -v.z, v.y), step(abs(v.x), abs(v.z)));
}

f32vec3 around(f32vec3 v, f32vec3 z) {
    f32vec3 t = ortho(z), b = cross(z, t);
    return t * f32vec3(v.x, v.x, v.x) + (b * f32vec3(v.y, v.y, v.y) + (z * v.z));
}

f32vec3 isotropic(f32 rp, f32 c) {
    f32 p = 2 * 3.14159 * rp, s = sqrt(1.0 - c * c);
    return f32vec3(cos(p) * s, sin(p) * s, c);
}

f32vec3 rand_pt(f32vec3 n, f32vec2 rnd) {
    f32 c = sqrt(rnd.y);
    return around(isotropic(rnd.x, c), n);
}

f32vec3 rand_pt_in_sphere(f32vec2 rnd) {
    f32 l = acos(2 * rnd.x - 1) - PI / 2;
    f32 p = 2 * PI * rnd.y;
    return f32vec3(cos(l) * cos(p), cos(l) * sin(p), sin(l));
}

f32vec3 rand_lambertian_nrm(f32vec3 n, f32vec2 rnd) {
    f32vec3 pt = rand_pt_in_sphere(rnd);
    return normalize(pt + n);
}

f32vec3 rand_lambertian_reflect(f32vec3 i, f32vec3 n, f32vec2 rnd, f32 roughness) {
    f32vec3 pt = rand_pt_in_sphere(rnd) * clamp(roughness, 0, 1);
    return normalize(pt + reflect(i, n));
}
