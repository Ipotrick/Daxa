#include "drawing/common.hlsl"
#include "utils/intersect.hlsl"

[numthreads(1, 1, 1)] void main() {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_sb);

    float3 front = (globals[0].viewproj_mat * float4(0, 0, 1, 0)).xyz;

    Ray view_ray;
    view_ray.o = globals[0].pos.xyz;
    view_ray.nrm = front;
    view_ray.inv_nrm = 1 / view_ray.nrm;
    RayIntersection view_chunk_intersection = trace_chunks(view_ray);
    float3 view_intersection_pos = get_intersection_pos_corrected(view_ray, view_chunk_intersection);

    globals[0].pick_pos = float4(view_intersection_pos, 0);
}
