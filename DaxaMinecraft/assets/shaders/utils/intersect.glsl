const uint SDF_MIN = 1000;

#include <utils/dda.glsl>

RayIntersection ray_box_intersect(in Ray ray, vec3 b_min, vec3 b_max) {
    RayIntersection result;
    float tx1 = (b_min.x - ray.o.x) * ray.inv_nrm.x;
    float tx2 = (b_max.x - ray.o.x) * ray.inv_nrm.x;
    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);
    float ty1 = (b_min.y - ray.o.y) * ray.inv_nrm.y;
    float ty2 = (b_max.y - ray.o.y) * ray.inv_nrm.y;
    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));
    float tz1 = (b_min.z - ray.o.z) * ray.inv_nrm.z;
    float tz2 = (b_max.z - ray.o.z) * ray.inv_nrm.z;
    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    result.hit = (tmax >= tmin && tmin > 0);
    result.dist = tmin;
    result.steps = 0;

    bool is_x = tmin == tx1 || tmin == tx2;
    bool is_y = tmin == ty1 || tmin == ty2;
    bool is_z = tmin == tz1 || tmin == tz2;

    if (is_z) {
        if (ray.nrm.z < 0) {
            result.nrm = vec3(0, 0, 1);
        } else {
            result.nrm = vec3(0, 0, -1);
        }
    } else if (is_y) {
        if (ray.nrm.y < 0) {
            result.nrm = vec3(0, 1, 0);
        } else {
            result.nrm = vec3(0, -1, 0);
        }
    } else {
        if (ray.nrm.x < 0) {
            result.nrm = vec3(1, 0, 0);
        } else {
            result.nrm = vec3(-1, 0, 0);
        }
    }

    return result;
}

vec3 get_intersection_pos(in Ray ray, in RayIntersection intersection) {
    return ray.o + (intersection.dist) * ray.nrm;
}

vec3 get_intersection_pos_corrected(in Ray ray, in RayIntersection intersection) {
    return get_intersection_pos(ray, intersection) - intersection.nrm * 0.001;
}

RayIntersection ray_step_voxels(in Ray ray, in vec3 b_min, in vec3 b_max, in uint max_steps) {
    RayIntersection result;
    result.hit = false;
    result.dist = 0;
    result.nrm = vec3(0);
    result.steps = 0;

    DDA_RunState dda_run_state;
    dda_run_state.outside_bounds = false;
    dda_run_state.is_y = false;
    dda_run_state.is_z = false;

    ivec3 tile_i = ivec3(ray.o);
    DDA_StartResult dda_start = run_dda_start(ray, dda_run_state);

    run_dda_main(dda_start, dda_run_state, b_min, b_max, max_steps);
    result.hit = dda_run_state.hit;

    if (dda_run_state.outside_bounds) {
        result.dist += 1.0f;
        result.hit = false;
    }

    float x = dda_run_state.to_side_dist_xy;
    float y = dda_run_state.to_side_dist_yx;
    float z = dda_run_state.to_side_dist_zx;

    float dx = dda_start.delta_dist.x;
    float dy = dda_start.delta_dist.y;
    float dz = dda_start.delta_dist.z;

    if (dda_run_state.is_z) {
        result.dist += z - dz;
        if (ray.nrm.z < 0) {
            result.nrm = vec3(0, 0, 1);
        } else {
            result.nrm = vec3(0, 0, -1);
        }
    } else if (dda_run_state.is_y) {
        result.dist += y - dy;
        if (ray.nrm.y < 0) {
            result.nrm = vec3(0, 1, 0);
        } else {
            result.nrm = vec3(0, -1, 0);
        }
    } else {
        result.dist += x - dx;
        if (ray.nrm.x < 0) {
            result.nrm = vec3(1, 0, 0);
        } else {
            result.nrm = vec3(-1, 0, 0);
        }
    }

    result.steps = dda_run_state.total_steps;

    return result;
}

RayIntersection trace_chunks(in Ray ray) {
    vec3 b_min = vec3(0), b_max = CHUNK_SIZE * CHUNK_N;
    RayIntersection result;
    result.hit = false;
    result.dist = 0;
    result.nrm = vec3(0);
    result.steps = 0;

    float sdf_dist_total = 0;
    uint sdf_step_total = 0;

    if (point_box_contains(ray.o, b_min, b_max)) {
        if (is_block_occluding(load_block_id(ray.o))) {
            result.hit = true;
            return result;
        }
    } else {
        RayIntersection bounds_intersection = ray_box_intersect(ray, b_min, b_max);
        if (!bounds_intersection.hit)
            return bounds_intersection;
        sdf_dist_total = bounds_intersection.dist;
        vec3 sample_pos = ray.o + ray.nrm * sdf_dist_total;
        if (!point_box_contains(sample_pos, b_min, b_max)) {
            sdf_dist_total += 0.0001 / abs(dot(ray.nrm, bounds_intersection.nrm));
            sample_pos = ray.o + ray.nrm * sdf_dist_total;
        }
        if (is_block_occluding(load_block_id(sample_pos))) {
            // if (load_block_presence_4x(sample_pos)) {
            // if (load_block_presence_16x(sample_pos)) {
            result.hit = true;
            result.nrm = bounds_intersection.nrm;
            result.dist = sdf_dist_total;
            return result;
        }
    }

    vec3 sample_pos = ray.o + ray.nrm * sdf_dist_total;
    Ray dda_ray = ray;
    dda_ray.o = sample_pos;
    RayIntersection dda_result = ray_step_voxels(dda_ray, vec3(0), CHUNK_SIZE * CHUNK_N, MAX_STEPS);
    sdf_dist_total += dda_result.dist;
    sdf_step_total += dda_result.steps;
    if (dda_result.hit) {
        result = dda_result;
    }

    // for (uint i = 0; i < 1000; ++i) {
    //     vec3 sample_pos = ray.o + ray.nrm * sdf_dist_total;
    //     if (!point_box_contains(sample_pos, b_min, b_max)) {
    //         result.hit = false;
    //         break;
    //     }
    //     uint tile = load_tile(sample_pos);
    //     uint sd_u = (tile & SDF_DIST_MASK) >> 0x18;
    //     if (sd_u < SDF_MIN) {
    //         Ray dda_ray = ray;
    //         dda_ray.o = sample_pos;
    //         RayIntersection dda_result = ray_step_voxels(dda_ray, vec3(0), CHUNK_SIZE * CHUNK_N, SDF_MIN + 4);
    //         sdf_dist_total += dda_result.dist;
    //         sdf_step_total += dda_result.steps;
    //         if (dda_result.hit) {
    //             result = dda_result;
    //             break;
    //         }
    //         sdf_dist_total -= 0.001;
    //     } else {
    //         float sd = float(sd_u - 3) * 0.57735;
    //         sdf_dist_total += sd;
    //     }
    //     ++sdf_step_total;
    // }

    result.dist = sdf_dist_total;
    result.steps = sdf_step_total;

    return result;
}
