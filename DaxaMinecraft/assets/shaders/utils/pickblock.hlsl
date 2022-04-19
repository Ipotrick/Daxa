#include "chunkgen/common.hlsl"
#include "utils/intersect.hlsl"

[numthreads(1, 1, 1)] void main() {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_sb);

    float3 front = mul(globals[0].viewproj_mat, float4(0, 0, 1, 0)).xyz;
    Ray ray;
    ray.o = GLOBALS_DEFINE.pos.xyz;
    ray.nrm = normalize(front);
    ray.inv_nrm = 1 / ray.nrm;
    // RayIntersection view_chunk_intersection = trace_chunks(GLOBALS_ARG ray);
    // float3 view_intersection_pos = get_intersection_pos_corrected(ray, view_chunk_intersection)

    float3 b_min = float3(0, 0, 0), b_max = float3(int(CHUNK_NX), int(CHUNK_NY), int(CHUNK_NZ)) * int(CHUNK_SIZE);
    DDA_RunState dda_run_state;
    dda_run_state.outside_bounds = false;
    dda_run_state.side = 0;
    DDA_StartResult dda_start = run_dda_start(ray, dda_run_state);
    run_dda_main(GLOBALS_ARG ray, dda_start, dda_run_state, b_min, b_max, 5);

    if (dda_run_state.hit)
        globals[0].pick_pos[0] = float4(0, 0, 0, 0);
}
