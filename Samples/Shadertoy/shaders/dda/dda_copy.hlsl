#pragma once

#include "drawing.hlsl"
#include "world.hlsl"

struct Ray {
    float2 o;
    float2 nrm;
    float2 inv_nrm;
};

void dda(in StructuredBuffer<Buf0> buf0, float2 pixel_pos, inout float3 color, Ray ray, int max_steps) {
    float2 delta = float2(
        ray.nrm.x == 0.0 ? 2.0 * max_steps : abs(ray.inv_nrm.x),
        ray.nrm.y == 0.0 ? 2.0 * max_steps : abs(ray.inv_nrm.y)
    );

    uint lod = get_lod(buf0, ray.o);
    if (lod == 0)
        return;
    float cell_size = float(1l << (lod - 1));

    float2 t_start;
    if (ray.nrm.x < 0) {
        t_start.x = (ray.o.x / cell_size - floor(ray.o.x / cell_size)) * cell_size * delta.x;
    } else {
        t_start.x = (ceil(ray.o.x / cell_size) - ray.o.x / cell_size) * cell_size * delta.x;
    }
    if (ray.nrm.y < 0) {
        t_start.y = (ray.o.y / cell_size - floor(ray.o.y / cell_size)) * cell_size * delta.y;
    } else {
        t_start.y = (ceil(ray.o.y / cell_size) - ray.o.y / cell_size) * cell_size * delta.y;
    }

    float t_curr = min(t_start.x, t_start.y);
    float2 current_pos;
    float2 t_next;

    for (int i = 0; i < max_steps; ++i) {
        current_pos = ray.o + ray.nrm * t_curr * 1.01;
        color_set(color, float3(1, 0.25, 1), circle_dist(pixel_pos, current_pos, 0.5) < 0);

        lod = get_lod(buf0, current_pos);
        if (lod == 0)
            break;
        cell_size = float(1l << (lod - 1));

        if (ray.nrm.x < 0) {
            t_next.x = (current_pos.x / cell_size - floor(current_pos.x / cell_size)) * cell_size * delta.x;
        } else {
            t_next.x = (ceil(current_pos.x / cell_size) - current_pos.x / cell_size) * cell_size * delta.x;
        }
        if (ray.nrm.y < 0) {
            t_next.y = (current_pos.y / cell_size - floor(current_pos.y / cell_size)) * cell_size * delta.y;
        } else {
            t_next.y = (ceil(current_pos.y / cell_size) - current_pos.y / cell_size) * cell_size * delta.y;
        }

        t_curr += min(t_next.x, t_next.y);
    }
}
