
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

vec3 get_intersection_pos(in Ray ray, in RayIntersection intersection) {
    return ray.o + (intersection.dist) * ray.nrm;
}

vec3 get_intersection_pos_corrected(in Ray ray, in RayIntersection intersection) {
    return get_intersection_pos(ray, intersection) - intersection.nrm * 0.001;
}
