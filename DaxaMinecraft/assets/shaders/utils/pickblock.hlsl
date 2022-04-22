#include "world/common.hlsl"
#include "utils/intersect.hlsl"

[numthreads(1, 1, 1)] void main() {
    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globals_sb);

    globals[0].player.update(globals[0].input);

    float3 front = mul(globals[0].viewproj_mat, float4(0, 0, 1, 0)).xyz;
    Ray ray;
    ray.o = globals[0].pos.xyz;
    ray.nrm = normalize(front);
    ray.inv_nrm = 1 / ray.nrm;
    RayIntersection view_chunk_intersection = trace_chunks(globals, ray);
    float3 view_intersection_pos0 = get_intersection_pos_corrected(ray, view_chunk_intersection);
    view_chunk_intersection.nrm *= -1;
    float3 view_intersection_pos1 = get_intersection_pos_corrected(ray, view_chunk_intersection);
    if (view_chunk_intersection.hit) {
        globals[0].pick_pos[0] = float4(view_intersection_pos0, 0);
        globals[0].pick_pos[1] = float4(view_intersection_pos1, 0);
    } else {
        globals[0].pick_pos[0] = float4(-100000, -100000, -100000, 0);
        globals[0].pick_pos[1] = globals[0].pick_pos[0];
    }
}
