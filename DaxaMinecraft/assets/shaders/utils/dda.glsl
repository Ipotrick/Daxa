#include <utils/ray.glsl>

struct DDA_StartResult {
    vec3 delta_dist;
    ivec3 ray_step;
};

struct DDA_RunState {
    float to_side_dist_xy, to_side_dist_xz;
    float to_side_dist_yx, to_side_dist_yz;
    float to_side_dist_zx, to_side_dist_zy;
    ivec3 tile_i;
    uint total_steps;
    bool hit, outside_bounds, is_y, is_z;
};

const float SCL = 4;

DDA_StartResult run_dda_start(in Ray ray, in out DDA_RunState run_state) {
    DDA_StartResult result;
    result.delta_dist = vec3(
        ray.nrm.x == 0 ? 1 : abs(ray.inv_nrm.x),
        ray.nrm.y == 0 ? 1 : abs(ray.inv_nrm.y),
        ray.nrm.z == 0 ? 1 : abs(ray.inv_nrm.z));
    run_state.tile_i = ivec3(ray.o / SCL) * ivec3(SCL);
    if (ray.nrm.x < 0) {
        result.ray_step.x = -1;
        run_state.to_side_dist_xy = (ray.o.x - run_state.tile_i.x) * result.delta_dist.x;
        run_state.to_side_dist_xz = (ray.o.x - run_state.tile_i.x) * result.delta_dist.x;
    } else {
        result.ray_step.x = 1;
        run_state.to_side_dist_xy = (run_state.tile_i.x + SCL - ray.o.x) * result.delta_dist.x;
        run_state.to_side_dist_xz = (run_state.tile_i.x + SCL - ray.o.x) * result.delta_dist.x;
    }
    if (ray.nrm.y < 0) {
        result.ray_step.y = -1;
        run_state.to_side_dist_yx = (ray.o.y - run_state.tile_i.y) * result.delta_dist.y;
        run_state.to_side_dist_yz = (ray.o.y - run_state.tile_i.y) * result.delta_dist.y;
    } else {
        result.ray_step.y = 1;
        run_state.to_side_dist_yx = (run_state.tile_i.y + SCL - ray.o.y) * result.delta_dist.y;
        run_state.to_side_dist_yz = (run_state.tile_i.y + SCL - ray.o.y) * result.delta_dist.y;
    }
    if (ray.nrm.z < 0) {
        result.ray_step.z = -1;
        run_state.to_side_dist_zx = (ray.o.z - run_state.tile_i.z) * result.delta_dist.z;
        run_state.to_side_dist_zy = (ray.o.z - run_state.tile_i.z) * result.delta_dist.z;
    } else {
        result.ray_step.z = 1;
        run_state.to_side_dist_zx = (run_state.tile_i.z + SCL - ray.o.z) * result.delta_dist.z;
        run_state.to_side_dist_zy = (run_state.tile_i.z + SCL - ray.o.z) * result.delta_dist.z;
    }
    result.delta_dist *= SCL;
    result.ray_step *= int(SCL);
    return result;
}

void run_dda_main(in DDA_StartResult dda_start, in out DDA_RunState run_state, in vec3 b_min, in vec3 b_max, in uint max_steps) {
    run_state.hit = false;
    for (run_state.total_steps = 0; run_state.total_steps < max_steps; ++run_state.total_steps) {
        uint tile = load_tile(run_state.tile_i);
        //if (is_block_occluding(get_block_id(tile))) {
        if (load_block_presence_4x(run_state.tile_i)) {
            // if (load_block_presence_16x(run_state.tile_i)) {
            run_state.hit = true;
            break;
        }
        if (run_state.to_side_dist_xy < run_state.to_side_dist_yx) {
            if (run_state.to_side_dist_xz < run_state.to_side_dist_zx) {
                run_state.to_side_dist_xy += dda_start.delta_dist.x;
                run_state.to_side_dist_xz += dda_start.delta_dist.x;
                run_state.tile_i.x += dda_start.ray_step.x;
                run_state.is_y = false, run_state.is_z = false;
            } else {
                run_state.to_side_dist_zx += dda_start.delta_dist.z;
                run_state.to_side_dist_zy += dda_start.delta_dist.z;
                run_state.tile_i.z += dda_start.ray_step.z;
                run_state.is_y = false, run_state.is_z = true;
            }
        } else {
            if (run_state.to_side_dist_yz < run_state.to_side_dist_zy) {
                run_state.to_side_dist_yx += dda_start.delta_dist.y;
                run_state.to_side_dist_yz += dda_start.delta_dist.y;
                run_state.tile_i.y += dda_start.ray_step.y;
                run_state.is_y = true, run_state.is_z = false;
            } else {
                run_state.to_side_dist_zx += dda_start.delta_dist.z;
                run_state.to_side_dist_zy += dda_start.delta_dist.z;
                run_state.tile_i.z += dda_start.ray_step.z;
                run_state.is_y = false, run_state.is_z = true;
            }
        }
        if (!point_box_contains(run_state.tile_i, b_min, b_max)) {
            run_state.outside_bounds = true;
            break;
        }
    }
}
