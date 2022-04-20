#include "chunkgen/common.hlsl"
#include "utils/intersect.hlsl"

[numthreads(1, 1, 1)] void main() {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_sb);

    float3 front = mul(globals[0].viewproj_mat, float4(0, 0, 1, 0)).xyz;
    Ray ray;
    ray.o = GLOBALS_DEFINE.pos.xyz;
    ray.nrm = normalize(front);
    ray.inv_nrm = 1 / ray.nrm;
    RayIntersection view_chunk_intersection = trace_chunks(GLOBALS_ARG ray);
    float3 view_intersection_pos = get_intersection_pos_corrected(ray, view_chunk_intersection);
    if (view_chunk_intersection.hit)
        globals[0].pick_pos[0] = float4(view_intersection_pos, 0);
    else
        globals[0].pick_pos[0] = float4(-100000, -100000, -100000, 0);
}
