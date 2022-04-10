#if !defined(INTERSECT_HLSL)
#define INTERSECT_HLSL

#include "utils/dda.hlsl"

RayIntersection ray_box_intersect(in Ray ray, float3 b_min, float3 b_max) {
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
            result.nrm = float3(0, 0, 1);
        } else {
            result.nrm = float3(0, 0, -1);
        }
    } else if (is_y) {
        if (ray.nrm.y < 0) {
            result.nrm = float3(0, 1, 0);
        } else {
            result.nrm = float3(0, -1, 0);
        }
    } else {
        if (ray.nrm.x < 0) {
            result.nrm = float3(1, 0, 0);
        } else {
            result.nrm = float3(-1, 0, 0);
        }
    }

    return result;
}

RayIntersection ray_wirebox_intersect(in Ray ray, float3 b_min, float3 b_max, float t) {
    RayIntersection result;
    result.hit = false;
    bool hit = false;
    float dist = 100000;
    float3 nrm;

    result = ray_box_intersect(ray, b_min, float3(b_min.x + t, b_min.y + t, b_max.z));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, b_min, float3(b_min.x + t, b_max.y, b_min.z + t));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, b_min, float3(b_max.x, b_min.y + t, b_min.z + t));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;

    result = ray_box_intersect(ray, float3(b_max.x - t, b_min.y, b_min.z), float3(b_max.x, b_min.y + t, b_max.z));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, float3(b_min.x, b_min.y, b_max.z - t), float3(b_min.x + t, b_max.y, b_max.z));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, float3(b_min.x, b_max.y - t, b_min.z), float3(b_max.x, b_max.y, b_min.z + t));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;

    result = ray_box_intersect(ray, b_max, float3(b_max.x - t, b_max.y - t, b_min.z));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, b_max, float3(b_max.x - t, b_min.y, b_max.z - t));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, b_max, float3(b_min.x, b_max.y - t, b_max.z - t));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;

    result = ray_box_intersect(ray, float3(b_min.x + t, b_max.y, b_max.z), float3(b_min.x, b_max.y - t, b_min.z));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, float3(b_max.x, b_max.y, b_min.z + t), float3(b_max.x - t, b_min.y, b_min.z));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;
    result = ray_box_intersect(ray, float3(b_max.x, b_min.y + t, b_max.z), float3(b_min.x, b_min.y, b_max.z - t));
    if (result.hit && result.dist < dist)
        hit = true, dist = result.dist, nrm = result.nrm;

    result.hit = hit;
    result.dist = dist;
    result.nrm = nrm;

    return result;
}

RayIntersection ray_sphere_intersect(in Ray ray, float3 s0, float sr) {
    RayIntersection result;
    result.hit = false;

    float a = dot(ray.nrm, ray.nrm);
    float3 s0_r0 = ray.o - s0;
    float b = 2.0 * dot(ray.nrm, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b * b - 4.0 * a * c < 0.0)
        return result;
    result.dist = (-b - sqrt((b * b) - 4.0 * a * c)) / (2.0 * a);
    result.hit = result.dist > 0;
    result.nrm = normalize(get_intersection_pos(ray, result) - s0);
    return result;
}

RayIntersection ray_cylinder_intersect(in Ray ray, in float3 pa, in float3 pb, float ra) {
    RayIntersection result;
    result.hit = false;

    float3 ba = pb - pa;

    float3 oc = ray.o - pa;

    float baba = dot(ba, ba);
    float bard = dot(ba, ray.nrm);
    float baoc = dot(ba, oc);

    float k2 = baba - bard * bard;
    float k1 = baba * dot(oc, ray.nrm) - baoc * bard;
    float k0 = baba * dot(oc, oc) - baoc * baoc - ra * ra * baba;

    float h = k1 * k1 - k2 * k0;
    if (h < 0.0)
        return result;
    h = sqrt(h);
    float t = (-k1 - h) / k2;

    // body
    float y = baoc + t * bard;
    if (y > 0.0 && y < baba) {

        result.dist = t;
        result.hit = t > 0;
        result.nrm = (oc + t * ray.nrm - ba * y / baba) / ra;
        return result;
    }

    // caps
    t = (((y < 0.0) ? 0.0 : baba) - baoc) / bard;
    if (abs(k1 + k2 * t) < h) {
        result.dist = t;
        result.hit = t > 0;
        result.nrm = ba * sign(y) / baba;
    }

    return result;
}

RayIntersection ray_step_voxels(in Ray ray, in float3 b_min, in float3 b_max, in uint max_steps) {
    RayIntersection result;
    result.hit = false;
    result.dist = 0;
    result.nrm = float3(0, 0, 0);
    result.steps = 0;

    DDA_RunState dda_run_state;
    dda_run_state.outside_bounds = false;
    dda_run_state.side = 0;

    int3 tile_i = int3(ray.o);
    DDA_StartResult dda_start = run_dda_start(ray, dda_run_state);

    run_dda_main(ray, dda_start, dda_run_state, b_min, b_max, max_steps);
    result.hit = dda_run_state.hit;

    if (dda_run_state.outside_bounds) {
        result.dist += 1.0f;
        result.hit = false;
    }

    float x = dda_run_state.to_side_dist.x;
    float y = dda_run_state.to_side_dist.y;
    float z = dda_run_state.to_side_dist.z;

#if VISUALIZE_SUBGRID == 0
    float dx = dda_start.delta_dist.x;
    float dy = dda_start.delta_dist.y;
    float dz = dda_start.delta_dist.z;
#else
    float dx = dda_start.delta_dist.x * VISUALIZE_SUBGRID;
    float dy = dda_start.delta_dist.y * VISUALIZE_SUBGRID;
    float dz = dda_start.delta_dist.z * VISUALIZE_SUBGRID;
#endif

    switch (dda_run_state.side) {
    case 0: result.dist += x - dx, result.nrm = float3(-dda_start.ray_step.x, 0, 0); break;
    case 1: result.dist += y - dy, result.nrm = float3(0, -dda_start.ray_step.y, 0); break;
    case 2: result.dist += z - dz, result.nrm = float3(0, 0, -dda_start.ray_step.z); break;
    }

    result.steps = dda_run_state.total_steps;

    return result;
}

RayIntersection trace_chunks(in Ray ray) {
    float3 b_min = float3(0, 0, 0), b_max = float3(int(CHUNK_NX), int(CHUNK_NY), int(CHUNK_NZ)) * int(CHUNK_SIZE);
    RayIntersection result;
    result.hit = false;
    result.dist = 0;
    result.nrm = float3(0, 0, 0);
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
        float3 sample_pos = ray.o + ray.nrm * sdf_dist_total;
        if (!point_box_contains(sample_pos, b_min, b_max)) {
            sdf_dist_total += 0.001;
            sample_pos = ray.o + ray.nrm * sdf_dist_total;
        }
#if VISUALIZE_SUBGRID == 1
        if (is_block_occluding(load_block_id(sample_pos))) {
#elif VISUALIZE_SUBGRID == 2
        if (load_block_presence_2x(sample_pos)) {
#elif VISUALIZE_SUBGRID == 4
        if (load_block_presence_4x(sample_pos)) {
#elif VISUALIZE_SUBGRID == 8
        if (load_block_presence_8x(sample_pos)) {
#elif VISUALIZE_SUBGRID == 16
        if (load_block_presence_16x(sample_pos)) {
#elif VISUALIZE_SUBGRID == 32
        if (load_block_presence_32x(sample_pos)) {
#elif VISUALIZE_SUBGRID == 64
        if (load_block_presence_64x(sample_pos)) {
#else
        if (is_block_occluding(load_block_id(sample_pos))) {
#endif
            result.hit = true;
            result.nrm = bounds_intersection.nrm;
            result.dist = sdf_dist_total;
            return result;
        }
    }

    float3 sample_pos = ray.o + ray.nrm * sdf_dist_total;
    Ray dda_ray = ray;
    dda_ray.o = sample_pos;
    RayIntersection dda_result = ray_step_voxels(dda_ray, b_min, b_max, int(MAX_STEPS));
    sdf_dist_total += dda_result.dist;
    sdf_step_total += dda_result.steps;
    if (dda_result.hit) {
        result = dda_result;
    }

    result.dist = sdf_dist_total;
    result.steps = sdf_step_total;

    return result;
}

#endif