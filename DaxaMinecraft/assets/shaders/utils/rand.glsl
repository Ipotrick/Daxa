
// Pseudo-random values in half-open range [0:1].
uint rand_hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}
uint rand_hash(uvec2 v) { return rand_hash(v.x ^ rand_hash(v.y)); }
uint rand_hash(uvec3 v) { return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z)); }
uint rand_hash(uvec4 v) { return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z) ^ rand_hash(v.w)); }
float rand_float_construct(uint m) {
    const uint ieee_mantissa = 0x007FFFFFu;
    const uint ieee_one = 0x3F800000u;
    m &= ieee_mantissa;
    m |= ieee_one;
    float f = uintBitsToFloat(m);
    return f - 1.0;
}
float rand(float x) { return rand_float_construct(rand_hash(floatBitsToUint(x))); }
float rand(vec2 v) { return rand_float_construct(rand_hash(floatBitsToUint(v))); }
float rand(vec3 v) { return rand_float_construct(rand_hash(floatBitsToUint(v))); }
float rand(vec4 v) { return rand_float_construct(rand_hash(floatBitsToUint(v))); }
vec2 rand_vec2(vec2 v) { return vec2(rand(v.x), rand(v.y)); }
vec3 rand_vec3(vec3 v) { return vec3(rand(v.x), rand(v.y), rand(v.z)); }
vec4 rand_vec4(vec4 v) { return vec4(rand(v.x), rand(v.y), rand(v.z), rand(v.w)); }
