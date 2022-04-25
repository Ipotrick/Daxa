
#include "drawing/common.hlsl"
#include "utils/intersect.hlsl"
#include "utils/noise.hlsl"
#include "chunk.hlsl"

#include "player.hlsl"

DAXA_DEFINE_BA_TEXTURE2DARRAY(float4)
DAXA_DEFINE_BA_SAMPLER(void)

uint tile_texture_index(StructuredBuffer<Globals> globals, BlockID block_id, BlockFace face) {
    // clang-format off
    switch (block_id) {
    case BlockID::Debug:           return 0;
    case BlockID::Air:             return 1;
    case BlockID::Bedrock:         return 2;
    case BlockID::Brick:           return 3;
    case BlockID::Cactus:          return 4;
    case BlockID::Cobblestone:     return 5;
    case BlockID::CompressedStone: return 6;
    case BlockID::DiamondOre:      return 7;
    case BlockID::Dirt:            return 8;
    case BlockID::DriedShrub:      return 9;
    case BlockID::Grass:
        switch (face) {
        case BlockFace::Back:
        case BlockFace::Front:
        case BlockFace::Left:
        case BlockFace::Right:     return 10;
        case BlockFace::Bottom:    return 8;
        case BlockFace::Top:       return 11;
        default:                   return 0;
        }
    case BlockID::Gravel:          return 12;
    case BlockID::Lava:            return 13 + int(globals[0].time * 6) % 8;
    case BlockID::Leaves:          return 21;
    case BlockID::Log:
        switch (face) {
        case BlockFace::Back:
        case BlockFace::Front:
        case BlockFace::Left:
        case BlockFace::Right:     return 22;
        case BlockFace::Bottom:
        case BlockFace::Top:       return 23;
        default:                   return 0;
        }
    case BlockID::MoltenRock:      return 24;
    case BlockID::Planks:          return 25;
    case BlockID::Rose:            return 26;
    case BlockID::Sand:            return 27;
    case BlockID::Sandstone:       return 28;
    case BlockID::Stone:           return 29;
    case BlockID::TallGrass:       return 30;
    case BlockID::Water:           return 31;
    default:                       return 0;
    }
    // clang-format on
}

void get_texture_info(in StructuredBuffer<Globals> globals, in RayIntersection ray_chunk_intersection, in float3 intersection_pos, in BlockID block_id, in out float2 tex_uv, in out BlockFace face_id, in out uint tex_id) {
    if (ray_chunk_intersection.nrm.x > 0.5) {
        face_id = BlockFace::Left;
        tex_uv = frac(intersection_pos.zy);
        tex_uv.y = 1 - tex_uv.y;
    } else if (ray_chunk_intersection.nrm.x < -0.5) {
        face_id = BlockFace::Right;
        tex_uv = frac(intersection_pos.zy);
        tex_uv = 1 - tex_uv;
    }
    if (ray_chunk_intersection.nrm.y > 0.5) {
        face_id = BlockFace::Bottom;
        tex_uv = frac(intersection_pos.xz);
        tex_uv.x = 1 - tex_uv.x;
    } else if (ray_chunk_intersection.nrm.y < -0.5) {
        face_id = BlockFace::Top;
        tex_uv = frac(intersection_pos.xz);
    }
    if (ray_chunk_intersection.nrm.z > 0.5) {
        face_id = BlockFace::Front;
        tex_uv = frac(intersection_pos.xy);
        tex_uv = 1 - tex_uv;
    } else if (ray_chunk_intersection.nrm.z < -0.5) {
        face_id = BlockFace::Back;
        tex_uv = frac(intersection_pos.xy);
        tex_uv.y = 1 - tex_uv.y;
    }

    tex_id = tile_texture_index(globals, block_id, face_id);
}

#include "drawing/ui.hlsl"
#include "drawing/world.hlsl"

[numthreads(8, 8, 1)] void main(uint3 pixel_i
                                : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globals_sb);
    StructuredBuffer<PlayerBuffer> player_buffer = daxa::getBuffer<PlayerBuffer>(p.player_buf_id);

    if (pixel_i.x >= globals[0].frame_dim.x ||
        pixel_i.y >= globals[0].frame_dim.y)
        return;

    const float2 subsamples = float2(SUBSAMPLE_N, SUBSAMPLE_N);
    const float2 inv_subsamples = 1 / subsamples;
    float2 inv_frame_dim = 1 / float2(globals[0].frame_dim);
    float aspect = float(globals[0].frame_dim.x) * inv_frame_dim.y; // * 0.5;
    int2 i_uv = int2(pixel_i.xy); // * int2(2, 1);
    // if (pixel_i.x < globals[0].frame_dim.x / 2) {
    // } else {
    //     i_uv.x -= globals[0].frame_dim.x;
    // }
    float uv_rand_offset = globals[0].time;
    float3 color = float3(0, 0, 0);
    float depth = 100000;

    float2 uv_offset =
#if JITTER_VIEW || ENABLE_TAA
        // (pixel_i.x > globals[0].frame_dim.x / 2) *
        float2(rand(float2(i_uv + uv_rand_offset + 10)),
               rand(float2(i_uv + uv_rand_offset)));
#else
        float2(0, 0);
#endif
    float2 uv = (float2(i_uv) + uv_offset * inv_subsamples) * inv_frame_dim * 2 - 1;

    if (!draw_ui(globals, uv, aspect, color)) {
        draw_world(globals, player_buffer, uv, pixel_i, subsamples, inv_subsamples, inv_frame_dim, aspect, color, depth);
    }
    
    draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 2 - 0, globals[0].frame_dim.y / 2 - 4, 1, 9);
    draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 2 - 4, globals[0].frame_dim.y / 2 - 0, 9, 1);

    // draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 4 - 0, globals[0].frame_dim.y / 2 - 4, 1, 9);
    // draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 4 - 4, globals[0].frame_dim.y / 2 - 0, 9, 1);
    // draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 4 + globals[0].frame_dim.x / 2 - 0, globals[0].frame_dim.y / 2 - 4, 1, 9);
    // draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 4 + globals[0].frame_dim.x / 2 - 4, globals[0].frame_dim.y / 2 - 0, 9, 1);

    // RayIntersection temp_inter;
    // for (int yi = 0; yi < 10; ++yi)
    //     for (int xi = 0; xi < 10; ++xi) {
    //         temp_inter = ray_sphere_intersect(cam_ray, float3(512 + xi * 2, 0, 512 + yi * 2), 1);
    //         draw(color, depth, getTexture2DArray<float4>(globals[0].texture_index).Load(int4((temp_inter.nrm.x * 0.5 + 0.5) * 16, (temp_inter.nrm.z * 0.5 + 0.5) * 16, int(GLOBALS_DEFINE.time * 5) % 30, 0)).rgb, temp_inter.dist, temp_inter.hit);
    //     }

    RWTexture2D<float4> output_image = daxa::getRWTexture2D<float4>(p.output_image_i);
    float4 prev_val = output_image[pixel_i.xy];
    float4 new_val = float4(pow(color, float3(1, 1, 1)), 1);
#if ENABLE_TAA
    if (pixel_i.x > globals[0].frame_dim.x / 2) {
        output_image[pixel_i.xy] = prev_val * 0.8 + new_val * 0.2;
    } else {
#endif
    output_image[pixel_i.xy] = new_val;
#if ENABLE_TAA
    }
#endif
}
