#pragma once

#define PI 3.14159265

f32 deg2rad(f32 d) {
    return d * PI / 180.0;
}

f32 rad2deg(f32 r) {
    return r * 180.0 / PI;
}

f32vec3 rgb2hsv(f32vec3 c) {
    f32vec4 K = f32vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    f32vec4 p = mix(f32vec4(c.bg, K.wz), f32vec4(c.gb, K.xy), step(c.b, c.g));
    f32vec4 q = mix(f32vec4(p.xyw, c.r), f32vec4(c.r, p.yzx), step(p.x, c.r));
    f32 d = q.x - min(q.w, q.y);
    f32 e = 1.0e-10;
    return f32vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

f32vec3 hsv2rgb(f32vec3 c) {
    f32vec4 k = f32vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    f32vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
    return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

f32vec4 uint_to_float4(u32 u) {
    f32vec4 result;
    result.r = f32((u >> 0x00) & 0xff) / 255;
    result.g = f32((u >> 0x08) & 0xff) / 255;
    result.b = f32((u >> 0x10) & 0xff) / 255;
    result.a = f32((u >> 0x18) & 0xff) / 255;
    return result;
}

u32 float4_to_uint(f32vec4 f) {
    u32 result = 0;
    result |= u32(clamp(f.r, 0, 1) * 255) << 0x00;
    result |= u32(clamp(f.g, 0, 1) * 255) << 0x08;
    result |= u32(clamp(f.b, 0, 1) * 255) << 0x10;
    result |= u32(clamp(f.a, 0, 1) * 255) << 0x18;
    return result;
}
