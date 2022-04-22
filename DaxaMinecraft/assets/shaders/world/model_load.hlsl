#include "world/common.hlsl"
#include "core.hlsl"

[numthreads(8, 8, 8)] void main(uint3 global_i
                                : SV_DispatchThreadID) {
    float3 block_pos =
        float3(global_i) + p.pos.xyz - float3(BLOCK_NX, BLOCK_NY, BLOCK_NZ) / 2;

    uint chunk_texture_id = p.output_image_i;
    RWTexture3D<uint> chunk = daxa::getRWTexture3D<uint>(chunk_texture_id);

    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globals_sb);
    StructuredBuffer<ModelLoadBuffer> model = daxa::getBuffer<ModelLoadBuffer>(globals[0].model_load_index);

    float3 m_pos = model[0].pos.xyz;
    float3 m_dim = model[0].dim.xyz;

    if (block_pos.x >= m_pos.x && block_pos.x < m_pos.x + m_dim.x &&
        block_pos.y >= m_pos.y && block_pos.y < m_pos.y + m_dim.y &&
        block_pos.z >= m_pos.z && block_pos.z < m_pos.z + m_dim.z) {
        int3 model_tile_i = int3(block_pos - m_pos);
        uint model_tile_index = model_tile_i.x + model_tile_i.y * 128 + model_tile_i.z * 128 * 128;
        BlockID model_tile = (BlockID)model[0].data[model_tile_index];
        if (model_tile != BlockID::Air)
            chunk[int3(global_i)] = (uint)model_tile;
    }
}
