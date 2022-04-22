#pragma once

#include "drawing.hlsl"
#include "world.hlsl"

#define N_DIV 6
// #define DIV 2

struct Ray {
    float2 o;
    float2 nrm;
    float2 inv_nrm;
};

struct DDA_StartState_Subdiv {
    float2 initial_toaxis_dist;
    int2 tile_i;
};

struct DDA_StartState {
    float2 delta_dist;
    int2 tile_step;

    DDA_StartState_Subdiv div[N_DIV];
};

struct DDA_RunState_Subdiv {
    int2 total_axis_steps;
};

struct DDA_RunState {
    int axis;
    int total_steps;
    bool hit;

    DDA_RunState_Subdiv div[N_DIV];
};

template <uint DIV, bool parent_check_break>
void run_dda_step(in StructuredBuffer<Buf0> buf0, float2 pixel_pos, inout float3 color, Ray ray, int max_steps, inout DDA_StartState start_state, inout DDA_RunState run_state, in int2 parent_tile_i) {
    for (int i = 0; i < max_steps; ++i) {
        int2 current_tile_i = start_state.div[DIV].tile_i + start_state.tile_step * (1l << DIV) * run_state.div[DIV].total_axis_steps;
        float s = square_dist<DIV>(pixel_pos, current_tile_i, 1l << DIV);
        color_add(color, get_color<DIV>(), s < 0 && s > -0.5);
        if (check_subtile_presence<(1l << DIV)>(buf0, current_tile_i)) {
            int2 tas = run_state.div[DIV].total_axis_steps;
            tas[run_state.axis] -= 1;
            float2 total_dist = start_state.div[DIV].initial_toaxis_dist + start_state.delta_dist * (1l << DIV) * tas;
            float2 hit_p = ray.o + total_dist[run_state.axis] * ray.nrm;
            hit_p[run_state.axis] += 0.001 * start_state.tile_step[run_state.axis];
            int2 tile_i_x0 = get_tile_i<1l << (DIV - 1)>(hit_p);
            run_state.div[uint(DIV) - 1].total_axis_steps = abs(tile_i_x0 - start_state.div[uint(DIV) - 1].tile_i) / (1l << (DIV - 1));
            run_dda_step<DIV - 1, true>(buf0, pixel_pos, color, ray, 4, start_state, run_state, current_tile_i);
            if (run_state.hit)
                break;
        }
        if (parent_check_break) {
            int2 cip = get_tile_i<1l << (DIV + 1)>(float2(current_tile_i) + 0.5);
            if (cip.x != parent_tile_i.x || cip.y != parent_tile_i.y)
                break;
        }
        float2 total_dist = start_state.div[DIV].initial_toaxis_dist + start_state.delta_dist * (1l << DIV) * run_state.div[DIV].total_axis_steps;
        if (total_dist.x < total_dist.y) {
            run_state.axis = 0;
        } else {
            run_state.axis = 1;
        }
        float2 hit_p = ray.o + total_dist[run_state.axis] * ray.nrm;
        run_state.div[DIV].total_axis_steps[run_state.axis] += 1;
    }
}

template <>
void run_dda_step<0, true>(in StructuredBuffer<Buf0> buf0, float2 pixel_pos, inout float3 color, Ray ray, int max_steps, inout DDA_StartState start_state, inout DDA_RunState run_state, in int2 parent_tile_i) {
    for (int i = 0; i < max_steps; ++i) {
        int2 current_tile_i = start_state.div[0].tile_i + start_state.tile_step * (1l << 0) * run_state.div[0].total_axis_steps;
        float s = square_dist<0>(pixel_pos, current_tile_i, 1l << 0);
        color_add(color, get_color<0>(), s < 0 && s > -0.5);
        if (check_subtile_presence<1>(buf0, current_tile_i)) {
            run_state.hit = true;
            break;
        }
        int2 cip = get_tile_i<2>(float2(current_tile_i) + 0.5);
        if (cip.x != parent_tile_i.x || cip.y != parent_tile_i.y) {
            break;
        }
        float2 total_dist = start_state.div[0].initial_toaxis_dist + start_state.delta_dist * (1l << 0) * run_state.div[0].total_axis_steps;
        if (total_dist.x < total_dist.y) {
            run_state.axis = 0;
        } else {
            run_state.axis = 1;
        }
        float2 hit_p = ray.o + total_dist[run_state.axis] * ray.nrm;
        run_state.div[0].total_axis_steps[run_state.axis] += 1;
    }
}

template <>
void run_dda_step<0, false>(in StructuredBuffer<Buf0> buf0, float2 pixel_pos, inout float3 color, Ray ray, int max_steps, inout DDA_StartState start_state, inout DDA_RunState run_state, in int2 parent_tile_i) {
    for (int i = 0; i < max_steps; ++i) {
        int2 current_tile_i = start_state.div[0].tile_i + start_state.tile_step * (1l << 0) * run_state.div[0].total_axis_steps;
        if (check_subtile_presence<1>(buf0, current_tile_i)) {
            run_state.hit = true;
            break;
        }
        float2 total_dist = start_state.div[0].initial_toaxis_dist + start_state.delta_dist * (1l << 0) * run_state.div[0].total_axis_steps;
        if (total_dist.x < total_dist.y) {
            run_state.axis = 0;
        } else {
            run_state.axis = 1;
        }
        float2 hit_p = ray.o + total_dist[run_state.axis] * ray.nrm;
        run_state.div[0].total_axis_steps[run_state.axis] += 1;
    }
}

void dda(in StructuredBuffer<Buf0> buf0, float2 pixel_pos, inout float3 color, Ray ray, int max_steps) {
    DDA_StartState start_state;
    start_state.delta_dist = float2(ray.nrm.x == 0 ? max_steps : abs(ray.inv_nrm.x), ray.nrm.y == 0 ? max_steps : abs(ray.inv_nrm.y));
    start_state.div[0].tile_i = get_tile_i<1>(ray.o);
    start_state.div[1].tile_i = get_tile_i<2>(ray.o);
    start_state.div[2].tile_i = get_tile_i<4>(ray.o);
    start_state.div[3].tile_i = get_tile_i<8>(ray.o);
    start_state.div[4].tile_i = get_tile_i<16>(ray.o);
    start_state.div[5].tile_i = get_tile_i<32>(ray.o);
    for (uint axis_i = 0; axis_i < 2; ++axis_i) {
        if (ray.nrm[axis_i] < 0) {
            start_state.tile_step[axis_i] = -1;
            for (uint div_i = 0; div_i < N_DIV; ++div_i)
                start_state.div[div_i].initial_toaxis_dist[axis_i] = (ray.o[axis_i] - start_state.div[div_i].tile_i[axis_i]) * start_state.delta_dist[axis_i];
        } else {
            start_state.tile_step[axis_i] = 1;
            for (uint div_i = 0; div_i < N_DIV; ++div_i)
                start_state.div[div_i].initial_toaxis_dist[axis_i] = (start_state.div[div_i].tile_i[axis_i] + (1l << div_i) - ray.o[axis_i]) * start_state.delta_dist[axis_i];
        }
    }

    DDA_RunState run_state;
    run_state.axis = 0;
    run_state.total_steps = 0;
    run_state.hit = false;
    for (uint div_i = 0; div_i < N_DIV; ++div_i)
        run_state.div[div_i].total_axis_steps = int2(0, 0);

    // run_dda_step<0, true>(pixel_pos, color, ray, 5, start_state, run_state, start_state.div[1].tile_i);
    // if (!run_state.hit) {
    //     run_state.div[1].total_axis_steps[run_state.axis] += 1;
    //     run_dda_step<1, true>(pixel_pos, color, ray, 5, start_state, run_state, start_state.div[2].tile_i);
    //     if (!run_state.hit) {
    //         run_state.div[2].total_axis_steps[run_state.axis] += 1;
    //         run_dda_step<2, true>(pixel_pos, color, ray, 5, start_state, run_state, start_state.div[3].tile_i);
    //         if (!run_state.hit) {
    //             run_state.div[3].total_axis_steps[run_state.axis] += 1;
    //             run_dda_step<3, true>(pixel_pos, color, ray, 5, start_state, run_state, start_state.div[4].tile_i);
    //             if (!run_state.hit) {
    //                 run_state.div[4].total_axis_steps[run_state.axis] += 1;
    //                 run_dda_step<4, true>(pixel_pos, color, ray, 5, start_state, run_state, start_state.div[5].tile_i);
    //                 if (!run_state.hit) {
    //                     run_state.div[5].total_axis_steps[run_state.axis] += 1;
    //                     run_dda_step<5, false>(pixel_pos, color, ray, max_steps, start_state, run_state, int2(0, 0));
    //                 }
    //             }
    //         }
    //     }
    // }
    run_dda_step<5, false>(buf0, pixel_pos, color, ray, max_steps, start_state, run_state, int2(0, 0));

    if (run_state.hit) {
        float dist = start_state.div[0].initial_toaxis_dist[run_state.axis] + start_state.delta_dist[run_state.axis] * (run_state.div[0].total_axis_steps[run_state.axis] - 1);
        float2 hit_p = ray.o + dist * ray.nrm;
        color_set(color, float3(0.9, 0.2, 0.9), circle_dist(pixel_pos, hit_p, 0.125) < 0);
    }
}