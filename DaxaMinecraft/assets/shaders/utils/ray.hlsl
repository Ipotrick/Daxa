#if !defined(RAY_HLSL)
#define RAY_HLSL

struct Ray {
    float3 o;
    float3 nrm, inv_nrm;
};

struct RayIntersection {
    bool hit;
    float dist;
    uint steps;
    float3 nrm;
};

bool point_box_contains(float3 p, float3 b_min, float3 b_max) {
    return (p.x >= b_min.x && p.x < b_max.x &&
            p.y >= b_min.y && p.y < b_max.y &&
            p.z >= b_min.z && p.z < b_max.z);
}

float3 get_intersection_pos(in Ray ray, in RayIntersection intersection) {
    return ray.o + (intersection.dist) * ray.nrm;
}

float3 get_intersection_pos_corrected(in Ray ray, in RayIntersection intersection) {
    return get_intersection_pos(ray, intersection) - intersection.nrm * 0.001;
}

#endif