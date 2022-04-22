#pragma once

uint rand_hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}
uint rand_hash(uint2 v) { return rand_hash(v.x ^ rand_hash(v.y)); }
uint rand_hash(uint3 v) {
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z));
}
uint rand_hash(uint4 v) {
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z) ^ rand_hash(v.w));
}
float rand_float_construct(uint m) {
    const uint ieee_mantissa = 0x007FFFFFu;
    const uint ieee_one = 0x3F800000u;
    m &= ieee_mantissa;
    m |= ieee_one;
    float f = asfloat(m);
    return f - 1.0;
}
float rand(float x) { return rand_float_construct(rand_hash(asuint(x))); }
float rand(float2 v) { return rand_float_construct(rand_hash(asuint(v))); }
float rand(float3 v) { return rand_float_construct(rand_hash(asuint(v))); }
float rand(float4 v) { return rand_float_construct(rand_hash(asuint(v))); }
float2 rand_vec2(float2 v) { return float2(rand(v.x), rand(v.y)); }
float3 rand_vec3(float3 v) { return float3(rand(v.x), rand(v.y), rand(v.z)); }
float4 rand_vec4(float4 v) {
    return float4(rand(v.x), rand(v.y), rand(v.z), rand(v.w));
}

float3 ortho(float3 v) {
    return lerp(float3(-v.y, v.x, 0.0), float3(0.0, -v.z, v.y), step(abs(v.x), abs(v.z)));
}

float3 around(float3 v, float3 z) {
    float3 t = ortho(z), b = cross(z, t);
    return mad(t, float3(v.x, v.x, v.x), mad(b, float3(v.y, v.y, v.y), z * v.z));
}

float3 isotropic(float rp, float c) {
    // sin(a) = sqrt(1.0 - cos(a)^2) , in the interval [0, PI/2] relevant for us
    float p = 2 * 3.14159 * rp, s = sqrt(1.0 - c * c);
    return float3(cos(p) * s, sin(p) * s, c);
}

float3 rand_pt(float3 n, float2 rnd) {
    float c = sqrt(rnd.y);
    return around(isotropic(rnd.x, c), n);
}
