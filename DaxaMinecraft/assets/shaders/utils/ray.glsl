
struct Ray {
    vec3 o;
    vec3 nrm, inv_nrm;
};

struct RayIntersection {
    bool hit;
    float dist;
    uint steps;

    vec3 nrm;
};

bool point_box_contains(vec3 p, vec3 b_min, vec3 b_max) {
    return (p.x >= b_min.x && p.x < b_max.x &&
            p.y >= b_min.y && p.y < b_max.y &&
            p.z >= b_min.z && p.z < b_max.z);
}

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
    return ray.o + intersection.dist * ray.nrm;
}

RayIntersection ray_step_voxels(in Ray ray, in vec3 b_min, in vec3 b_max) {
    RayIntersection result;
    result.hit = false;
    result.dist = 0;
    result.nrm = vec3(0);
    result.steps = 0;

    if (point_box_contains(ray.o, b_min, b_max)) {
        if (is_voxel_occluding(ray.o)) {
            result.hit = true;
            return result;
        }
        result.dist = 0;
    } else {
        RayIntersection bounds_intersection = ray_box_intersect(ray, b_min, b_max);
        if (!bounds_intersection.hit)
            return bounds_intersection;
        ray.o = get_intersection_pos(ray, bounds_intersection);
        result.dist = bounds_intersection.dist + 0.001;
        if (!point_box_contains(ray.o, b_min, b_max)) {
            ray.o += ray.nrm * 0.001;
        }
        if (is_voxel_occluding(ray.o)) {
            result.hit = true;
            result.nrm = bounds_intersection.nrm;
            return result;
        }
    }

    ivec3 tile_i = ivec3(ray.o);

    const float delta_dist_x = ray.nrm.x == 0 ? 1 : abs(ray.inv_nrm.x);
    const float delta_dist_y = ray.nrm.y == 0 ? 1 : abs(ray.inv_nrm.y);
    const float delta_dist_z = ray.nrm.z == 0 ? 1 : abs(ray.inv_nrm.z);

    float to_side_dist_xy;
    float to_side_dist_xz;
    float to_side_dist_yx;
    float to_side_dist_yz;
    float to_side_dist_zx;
    float to_side_dist_zy;

    ivec3 ray_step;

    if (ray.nrm.x < 0) {
        ray_step.x = -1;
        to_side_dist_xy = (ray.o.x - tile_i.x) * delta_dist_x;
        to_side_dist_xz = (ray.o.x - tile_i.x) * delta_dist_x;
    } else {
        ray_step.x = 1;
        to_side_dist_xy = (tile_i.x + 1.0f - ray.o.x) * delta_dist_x;
        to_side_dist_xz = (tile_i.x + 1.0f - ray.o.x) * delta_dist_x;
    }
    if (ray.nrm.y < 0) {
        ray_step.y = -1;
        to_side_dist_yx = (ray.o.y - tile_i.y) * delta_dist_y;
        to_side_dist_yz = (ray.o.y - tile_i.y) * delta_dist_y;
    } else {
        ray_step.y = 1;
        to_side_dist_yx = (tile_i.y + 1.0f - ray.o.y) * delta_dist_y;
        to_side_dist_yz = (tile_i.y + 1.0f - ray.o.y) * delta_dist_y;
    }
    if (ray.nrm.z < 0) {
        ray_step.z = -1;
        to_side_dist_zx = (ray.o.z - tile_i.z) * delta_dist_z;
        to_side_dist_zy = (ray.o.z - tile_i.z) * delta_dist_z;
    } else {
        ray_step.z = 1;
        to_side_dist_zx = (tile_i.z + 1.0f - ray.o.z) * delta_dist_z;
        to_side_dist_zy = (tile_i.z + 1.0f - ray.o.z) * delta_dist_z;
    }

    bool outside_bounds = false, is_y = false, is_z = false;
    uint total_steps;
    for (total_steps = 0; total_steps < MAX_STEPS; ++total_steps) {
        if (is_voxel_occluding(tile_i)) {
            result.hit = true;
            break;
        }
        if (to_side_dist_xy < to_side_dist_yx) {
            if (to_side_dist_xz < to_side_dist_zx) {
                to_side_dist_xy += delta_dist_x;
                to_side_dist_xz += delta_dist_x;
                tile_i.x += ray_step.x;
                is_y = false, is_z = false;
            } else {
                to_side_dist_zx += delta_dist_z;
                to_side_dist_zy += delta_dist_z;
                tile_i.z += ray_step.z;
                is_y = false, is_z = true;
            }
        } else {
            if (to_side_dist_yz < to_side_dist_zy) {
                to_side_dist_yx += delta_dist_y;
                to_side_dist_yz += delta_dist_y;
                tile_i.y += ray_step.y;
                is_y = true, is_z = false;
            } else {
                to_side_dist_zx += delta_dist_z;
                to_side_dist_zy += delta_dist_z;
                tile_i.z += ray_step.z;
                is_y = false, is_z = true;
            }
        }
        if (!point_box_contains(tile_i, b_min, b_max)) {
            outside_bounds = true;
            break;
        }
    }

    if (outside_bounds)
        result.hit = false;

    float x = to_side_dist_xy;
    float y = to_side_dist_yx;
    float z = to_side_dist_zx;

    if (is_z) {
        result.dist += z - delta_dist_z + 0.001;
        if (ray.nrm.z < 0) {
            result.nrm = vec3(0, 0, 1);
        } else {
            result.nrm = vec3(0, 0, -1);
        }
    } else if (is_y) {
        result.dist += y - delta_dist_y + 0.001;
        if (ray.nrm.y < 0) {
            result.nrm = vec3(0, 1, 0);
        } else {
            result.nrm = vec3(0, -1, 0);
        }
    } else {
        result.dist += x - delta_dist_x + 0.001;
        if (ray.nrm.x < 0) {
            result.nrm = vec3(1, 0, 0);
        } else {
            result.nrm = vec3(-1, 0, 0);
        }
    }

    result.steps = total_steps;

    return result;
}

RayIntersection trace_chunks(in Ray ray) {
    //return ray_step_voxels(ray, vec3(0), vec3(64) * CHUNK_N);

    vec3 b_min = vec3(0), b_max = vec3(64) * CHUNK_N;

    RayIntersection result;
    result.hit = false;
    result.dist = 0;
    result.nrm = vec3(0);
    result.steps = 0;

    if (point_box_contains(ray.o, b_min, b_max)) {
        if (is_voxel_occluding(ray.o)) {
            result.hit = true;
            return result;
        }
        result.dist = 0;
    } else {
        RayIntersection bounds_intersection = ray_box_intersect(ray, b_min, b_max);
        if (!bounds_intersection.hit)
            return bounds_intersection;
        ray.o = get_intersection_pos(ray, bounds_intersection);
        result.dist = bounds_intersection.dist + 0.001;
        if (!point_box_contains(ray.o, b_min, b_max)) {
            ray.o += ray.nrm * 0.001;
        }
        if (is_voxel_occluding(ray.o)) {
            result.hit = true;
            result.nrm = bounds_intersection.nrm;
            return result;
        }
    }

    float total_sdf_dist = 0;
    bool outside_bounds = false;

    float dst_factor = 0.5 * 1.4142;

    for (uint i = 0; i < 100; ++i) {
        vec3 sample_pos = ray.o + total_sdf_dist * ray.nrm;
        uint tile = get_tile(sample_pos);
        uint sdf = (tile & SDF_DIST_MASK) >> 0x18;
        total_sdf_dist += float(sdf) * dst_factor;
        if (sdf < 2) {
            result.hit = true;
            break;
        }
        if (!point_box_contains(sample_pos, b_min, b_max)) {
            outside_bounds = true;
            break;
        }
        ++result.steps;
    }

    if (result.hit) {
        result.dist += total_sdf_dist;
        RayIntersection dda_result = ray_step_voxels(ray, vec3(0), vec3(64) * CHUNK_N);
        dda_result.steps += result.steps;
        return dda_result;
    }

    return result;
}
