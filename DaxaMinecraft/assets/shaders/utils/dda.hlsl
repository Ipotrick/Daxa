#pragma once

#include "chunk.hlsl"
#include "utils/ray.hlsl"

struct DDA_RunState {
    uint side;
    uint total_steps;
    bool hit, outside_bounds;
    float dist;
};

uint get_lod(StructuredBuffer<Globals> globals, in float3 p) {
    BlockID block_id = load_block_id(globals, int3(p));
    if (is_block_occluding(block_id))
        return 0;
    if (x_load_presence<2>(globals, p))
        return 1;
    if (x_load_presence<4>(globals, p))
        return 2;
    if (x_load_presence<8>(globals, p))
        return 3;
    if (x_load_presence<16>(globals, p))
        return 4;
    if (x_load_presence<32>(globals, p))
        return 5;
    return 6;
}

void run_dda_main(StructuredBuffer<Globals> globals, in Ray ray, in out DDA_RunState run_state, in float3 b_min, in float3 b_max, in uint max_steps) {
    run_state.hit = false;
    uint x1_steps = 0;

    float3 delta = float3(
        ray.nrm.x == 0.0 ? 3.0 * max_steps : abs(ray.inv_nrm.x),
        ray.nrm.y == 0.0 ? 3.0 * max_steps : abs(ray.inv_nrm.y),
        ray.nrm.z == 0.0 ? 3.0 * max_steps : abs(ray.inv_nrm.z));

    uint lod = get_lod(globals, ray.o);
    if (lod == 0) {
        run_state.hit = true;
        return;
    }
    float cell_size = float(1l << (lod - 1));

    float3 t_start;
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
    if (ray.nrm.z < 0) {
        t_start.z = (ray.o.z / cell_size - floor(ray.o.z / cell_size)) * cell_size * delta.z;
    } else {
        t_start.z = (ceil(ray.o.z / cell_size) - ray.o.z / cell_size) * cell_size * delta.z;
    }

    float t_curr = min(min(t_start.x, t_start.y), t_start.z);
    float3 current_pos;
    float3 t_next = t_start;

    for (x1_steps = 0; x1_steps < max_steps; ++x1_steps) {
        current_pos = ray.o + ray.nrm * t_curr;
        if (!point_box_contains(current_pos, b_min, b_max)) {
            run_state.outside_bounds = true;
            break;
        }
        lod = get_lod(globals, current_pos);
        // if (lod < 0 + t_curr * 0.005) {
        if (lod == 0) {
            run_state.hit = true;
            if (t_next.x < t_next.y) {
                if (t_next.x < t_next.z) {
                    run_state.side = 0;
                } else {
                    run_state.side = 2;
                }
            } else {
                if (t_next.y < t_next.z) {
                    run_state.side = 1;
                } else {
                    run_state.side = 2;
                }
            }
            break;
        }
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
        if (ray.nrm.z < 0) {
            t_next.z = (current_pos.z / cell_size - floor(current_pos.z / cell_size)) * cell_size * delta.z;
        } else {
            t_next.z = (ceil(current_pos.z / cell_size) - current_pos.z / cell_size) * cell_size * delta.z;
        }

        t_curr += min(min(t_next.x, t_next.y), t_next.z) + 0.001;
    }

    run_state.total_steps = x1_steps;
    run_state.dist = t_curr;
}
