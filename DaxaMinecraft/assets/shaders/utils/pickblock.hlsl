#include "drawing/common.glsl"
#include "utils/intersect.glsl"

[numthreads(1, 1, 1)] void main() {
    float3 front = (globals.viewproj_mat * float4(0, 0, 1, 0)).xyz;

    Ray view_ray;
    view_ray.o = globals.pos.xyz;
    view_ray.nrm = front;
    view_ray.inv_nrm = 1 / view_ray.nrm;
    RayIntersection view_chunk_intersection = trace_chunks(view_ray);
    float3 view_intersection_pos = get_intersection_pos_corrected(view_ray, view_chunk_intersection);

    globals.pick_pos = float4(view_intersection_pos, 0);
}
