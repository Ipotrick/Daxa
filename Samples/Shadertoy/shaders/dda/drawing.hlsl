#pragma once

float grid_dist(float2 p, float s, float t) {
    // clang-format off
    return min(
        abs(frac(p.x * s - 0.5) - 0.5),
        abs(frac(p.y * s - 0.5) - 0.5)
    ) * 2 - t * s;
    // clang-format on
}

template<uint N>
float square_dist(float2 p, float2 c, float r) {
    // clang-format off
    return max(
        abs(c.x - p.x + 0.5 * (1l << N)),
        abs(c.y - p.y + 0.5 * (1l << N))
    ) * 2 - r;
    // clang-format on
}

float circle_dist(float2 p, float2 c, float r) { return length(p - c) - r; }

float line_segment_dist(float2 p, float2 a, float2 b, float r) {
    float2 ba = b - a;
    float2 pa = p - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
    return length(pa - h * ba) - r;
}

int manhattan_dist(int2 a, int2 b) {
    return abs(a.x - b.x) + abs(a.y - b.y);  //
}

void color_set(inout float3 color, float3 new_color, bool should) {
    if (should)
        color = new_color;
}
void color_add(inout float3 color, float3 new_color, bool should) {
    if (should)
        color += new_color;
}
void color_mix(inout float3 color, float3 new_color, float fac) {
    color = color * (1 - fac) + new_color * fac;  //
}
