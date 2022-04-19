struct Push {
    float4 pos;
    uint globals_sb;
    uint output_image_i;
    uint set_id;
};

[[vk::push_constant]] const Push p;

#include "core.hlsl"
#include "drawing/defines.hlsl"

[numthreads(8, 8, 8)] void main(uint3 global_i
                                : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_sb);
    float3 bp = float3(global_i) + p.pos.xyz;

    uint chunk_texture_id = p.output_image_i;
    RWTexture3D<uint> chunk = getRWTexture3D<uint>(chunk_texture_id);

    if (length(int3(bp) - int3(globals[0].pick_pos[0].xyz)) <= 0.5 + BLOCKEDIT_RADIUS) {
        chunk[int3(global_i)] = p.set_id;
    }
}
