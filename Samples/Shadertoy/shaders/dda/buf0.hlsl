#include "world.hlsl"

SHADERTOY_NUMTHREADS_BUF0 void buf0_main(uint3 tile_i
                                         : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_id);
    StructuredBuffer<Buf0> buf0 = getBuffer<Buf0>(p.buf0_id);

    switch (p.output_image_id) {
    case 0: {
        int i = tile_i.x + tile_i.y * CHUNK_SIZE / 1;
        FractalNoiseConfig noise_conf = {
            /* .amplitude   = */ 1.0f,
            /* .persistance = */ 0.4f,
            /* .scale       = */ 0.03f,
            /* .lacunarity  = */ 2,
            /* .octaves     = */ 6,
        };
        buf0[0].data_0[i] = fractal_noise(float3(tile_i.xy, 0), noise_conf) - 0.5 < 0 ? 1 : 0;
    } break;
    case 1: {
        int i = tile_i.x + tile_i.y * CHUNK_SIZE / 2;
        int2 ptile_i = tile_i.xy * 2;
        int pi0 = (ptile_i.x + 0) + (ptile_i.y + 0) * CHUNK_SIZE / 1;
        int pi1 = (ptile_i.x + 1) + (ptile_i.y + 0) * CHUNK_SIZE / 1;
        int pi2 = (ptile_i.x + 0) + (ptile_i.y + 1) * CHUNK_SIZE / 1;
        int pi3 = (ptile_i.x + 1) + (ptile_i.y + 1) * CHUNK_SIZE / 1;
        buf0[0].data_1[i] = buf0[0].data_0[pi0] > 0 || buf0[0].data_0[pi1] > 0 || buf0[0].data_0[pi2] > 0 || buf0[0].data_0[pi3] > 0;
    } break;
    case 2: {
        int i = tile_i.x + tile_i.y * CHUNK_SIZE / 4;
        int2 ptile_i = tile_i.xy * 2;
        int pi0 = (ptile_i.x + 0) + (ptile_i.y + 0) * CHUNK_SIZE / 2;
        int pi1 = (ptile_i.x + 1) + (ptile_i.y + 0) * CHUNK_SIZE / 2;
        int pi2 = (ptile_i.x + 0) + (ptile_i.y + 1) * CHUNK_SIZE / 2;
        int pi3 = (ptile_i.x + 1) + (ptile_i.y + 1) * CHUNK_SIZE / 2;
        buf0[0].data_2[i] = buf0[0].data_1[pi0] > 0 || buf0[0].data_1[pi1] > 0 || buf0[0].data_1[pi2] > 0 || buf0[0].data_1[pi3] > 0;
    } break;
    case 3: {
        int i = tile_i.x + tile_i.y * CHUNK_SIZE / 8;
        int2 ptile_i = tile_i.xy * 2;
        int pi0 = (ptile_i.x + 0) + (ptile_i.y + 0) * CHUNK_SIZE / 4;
        int pi1 = (ptile_i.x + 1) + (ptile_i.y + 0) * CHUNK_SIZE / 4;
        int pi2 = (ptile_i.x + 0) + (ptile_i.y + 1) * CHUNK_SIZE / 4;
        int pi3 = (ptile_i.x + 1) + (ptile_i.y + 1) * CHUNK_SIZE / 4;
        buf0[0].data_3[i] = buf0[0].data_2[pi0] > 0 || buf0[0].data_2[pi1] > 0 || buf0[0].data_2[pi2] > 0 || buf0[0].data_2[pi3] > 0;
    } break;
    case 4: {
        int i = tile_i.x + tile_i.y * CHUNK_SIZE / 16;
        int2 ptile_i = tile_i.xy * 2;
        int pi0 = (ptile_i.x + 0) + (ptile_i.y + 0) * CHUNK_SIZE / 8;
        int pi1 = (ptile_i.x + 1) + (ptile_i.y + 0) * CHUNK_SIZE / 8;
        int pi2 = (ptile_i.x + 0) + (ptile_i.y + 1) * CHUNK_SIZE / 8;
        int pi3 = (ptile_i.x + 1) + (ptile_i.y + 1) * CHUNK_SIZE / 8;
        buf0[0].data_4[i] = buf0[0].data_3[pi0] > 0 || buf0[0].data_3[pi1] > 0 || buf0[0].data_3[pi2] > 0 || buf0[0].data_3[pi3] > 0;
    } break;
    case 5: {
        int i = tile_i.x + tile_i.y * CHUNK_SIZE / 32;
        int2 ptile_i = tile_i.xy * 2;
        int pi0 = (ptile_i.x + 0) + (ptile_i.y + 0) * CHUNK_SIZE / 16;
        int pi1 = (ptile_i.x + 1) + (ptile_i.y + 0) * CHUNK_SIZE / 16;
        int pi2 = (ptile_i.x + 0) + (ptile_i.y + 1) * CHUNK_SIZE / 16;
        int pi3 = (ptile_i.x + 1) + (ptile_i.y + 1) * CHUNK_SIZE / 16;
        buf0[0].data_5[i] = buf0[0].data_4[pi0] > 0 || buf0[0].data_4[pi1] > 0 || buf0[0].data_4[pi2] > 0 || buf0[0].data_4[pi3] > 0;
    } break;
    }
}
