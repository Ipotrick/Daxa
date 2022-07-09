#pragma once

void draw_rect(in uint2 pixel_i, inout float3 color, int px, int py, int sx, int sy) {
    if (pixel_i.x >= px &&
        pixel_i.x < px + sx &&
        pixel_i.y >= py &&
        pixel_i.y < py + sy)
        color = float3(1, 1, 1);
}

void draw(inout float3 color, inout float depth, in float3 new_color, in float new_depth, bool should) {
    if (should && new_depth < depth) {
        color = new_color;
        depth = new_depth;
    }
}

void overlay(inout float3 color, inout float depth, in float3 new_color, in float new_depth, bool should, float fac) {
    if (should) {
        color = color * (1 - fac) + new_color * fac;
    }
}

struct Push {
    uint globals_sb;
    uint player_buf_id;
    uint output_image_i;
};

[[vk::push_constant]] const Push p;

#include "drawing/defines.hlsl"
