
#include "drawing/common.hlsl"
#include "utils/intersect.hlsl"
#include "utils/noise.hlsl"
#include "chunk.hlsl"

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

DAXA_DEFINE_BA_TEXTURE2DARRAY(float4)
DAXA_DEFINE_BA_SAMPLER(void)

#define sun_col float3(2, 1.7, 1)

float3 sample_sky(in Ray sun_ray, float3 nrm) {
    float sun_val = clamp((dot(nrm, sun_ray.nrm) - 0.9999) * 10000, 0, 1);
    float sky_val = (dot(nrm, float3(0, 1, 0))) / 2 + clamp((dot(nrm, sun_ray.nrm) + 1) * 0.1, 0, 1);
    return sun_col * sun_val + lerp(float3(0.5, 0.6, 2), float3(0.1, 0.2, 0.5), sky_val);
}

float3 shaded(in Ray sun_ray, in float3 col, in float3 nrm) {
    float3 light = sun_col * max(dot(nrm, sun_ray.nrm), 0) + sample_sky(sun_ray, nrm) * 0.6;
    return col * light;
}

[numthreads(8, 8, 1)] void main(uint3 pixel_i
                                : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = daxa::getBuffer<Globals>(p.globals_sb);

    if (pixel_i.x >= globals[0].frame_dim.x ||
        pixel_i.y >= globals[0].frame_dim.y)
        return;

    float3 front = mul(globals[0].viewproj_mat, float4(0, 0, 1, 0)).xyz;
    float3 right = mul(globals[0].viewproj_mat, float4(1, 0, 0, 0)).xyz;
    float3 up = mul(globals[0].viewproj_mat, float4(0, 1, 0, 0)).xyz;

    // float3x3 view_mat = globals[0].player.camera.view_mat;
    // float3 front = mul(view_mat, float3(0, 0, 1));
    // float3 right = mul(view_mat, float3(1, 0, 0));
    // float3 up = mul(view_mat, float3(0, 1, 0));

    float3 view_intersection_pos = globals[0].pick_pos[0].xyz;
    int3 view_intersection_block_pos = int3(view_intersection_pos);
    Ray cam_ray;
    cam_ray.o = globals[0].pos.xyz;
    // cam_ray.o = globals[0].player.pos;

    Ray sun_ray;
    float sun_angle = 0.3;
    float sun_yz = -abs(cos(sun_angle)) * 2;
    sun_ray.nrm = normalize(float3(sin(sun_angle) * 3, sun_yz, -0.2 * sun_yz));
    sun_ray.inv_nrm = 1 / sun_ray.nrm;

    const float2 subsamples = float2(SUBSAMPLE_N, SUBSAMPLE_N);
    const float2 inv_subsamples = 1 / subsamples;
    float2 inv_frame_dim = 1 / float2(globals[0].frame_dim);
    float aspect = float(globals[0].frame_dim.x) * inv_frame_dim.y;
    int2 i_uv = int2(pixel_i.xy);
    float uv_rand_offset = globals[0].time;
    float3 color = float3(0, 0, 0);
    float depth = 100000;

    float2 uv_offset =
#if JITTER_VIEW
        // (pixel_i.x > globals[0].frame_dim.x / 2) *
        float2(rand(float2(i_uv + uv_rand_offset + 10)),
               rand(float2(i_uv + uv_rand_offset)));
#else
        float2(0, 0);
#endif
    float2 uv = (float2(i_uv) + uv_offset * inv_subsamples) * inv_frame_dim * 2 - 1;

    for (uint yi = 0; yi < subsamples.y; ++yi) {
        for (uint xi = 0; xi < subsamples.x; ++xi) {
            float2 view_uv = (uv + inv_frame_dim * float2(xi, yi) * inv_subsamples) * globals[0].fov * float2(aspect, 1);

            cam_ray.nrm = normalize(front + view_uv.x * right + view_uv.y * up);
            cam_ray.inv_nrm = 1 / cam_ray.nrm;

            RayIntersection ray_chunk_intersection = trace_chunks(globals, cam_ray);
#if VISUALIZE_STEP_COMPLEXITY
            color.r += log(float(ray_chunk_intersection.steps) * 1 / MAX_STEPS + 1);
#else
            float3 intersection_pos = get_intersection_pos_corrected(cam_ray, ray_chunk_intersection);
            BlockID block_id = load_block_id(globals, intersection_pos);
            int3 intersection_block_pos = int3(intersection_pos);

            if (ray_chunk_intersection.hit) {
#if ENABLE_SHADOWS
                sun_ray.o = intersection_pos + ray_chunk_intersection.nrm * 0.002;
                RayIntersection sun_ray_chunk_intersection = trace_chunks(globals, sun_ray);
                float val = float(!sun_ray_chunk_intersection.hit);
                val = max(val * dot(ray_chunk_intersection.nrm, sun_ray.nrm), 0.01);
                float3 light = val * sun_col + sample_sky(sun_ray, ray_chunk_intersection.nrm) * 0.2;
#else
                // float3 light = shaded(sun_ray, float3(1, 1, 1), ray_chunk_intersection.nrm);
                float3 light = float3(1, 1, 1);
#endif
                float3 b_uv = float3(int3(intersection_pos) % 64) / 64;
                BlockFace face_id;
                float2 tex_uv = float2(0, 0);
                depth = ray_chunk_intersection.dist;

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

                uint tex_id = tile_texture_index(globals, block_id, face_id);

                if (tex_id == 8 || tex_id == 11) {
                    float r = rand(int3(intersection_pos));
                    switch (int(r * 4) % 4) {
                    case 0:
                        tex_uv = 1 - tex_uv;
                    case 1:
                        tex_uv = float2(tex_uv.y, tex_uv.x);
                        break;
                    case 2:
                        tex_uv = 1 - tex_uv;
                    case 3:
                        tex_uv = float2(tex_uv.x, tex_uv.y);
                        break;
                    default:
                        break;
                    }
                }

#if ALBEDO == ALBEDO_TEXTURE
                // WTF!!!
                // float3 albedo = daxa::getTexture2DArray<float4>(globals[0].texture_index).Sample(
                //     daxa::getSampler<void>(globals[0].sampler_index),
                //     float3(tex_uv.x, tex_uv.y, float(tex_id)),
                //     int2(0, 0),
                //     1.0f).rgb;
                float3 albedo = daxa::getTexture2DArray<float4>(globals[0].texture_index).Load(int4(tex_uv.x * 16, tex_uv.y * 16, tex_id, 0)).rgb;
#elif ALBEDO == ALBEDO_DEBUG_POS
                float3 albedo = b_uv;
#elif ALBEDO == ALBEDO_DEBUG_NRM
                float3 albedo = ray_chunk_intersection.nrm * 0.5 + 0.5;
#elif ALBEDO == ALBEDO_DEBUG_DIST
                float d = ray_chunk_intersection.dist;
                float3 albedo = float3(d, d, d) * 0.01;
#elif ALBEDO == ALBEDO_DEBUG_RANDOM
                float3 albedo = float3(rand(int3(intersection_pos)), rand(int3(intersection_pos + 10)), rand(int3(intersection_pos + 20)));
#endif
#if SHOW_PICK_POS
                if (length(intersection_block_pos - view_intersection_block_pos) <= 0.5 + BLOCKEDIT_RADIUS) {
                    float luminance = (albedo.r * 0.2126 + albedo.g * 0.7152 + albedo.b * 0.0722);
                    const float block_outline = 1.0 / 16;
                    albedo = float3(0.2, 0.5, 0.9) * 0.2 + luminance;
                    if (tex_uv.x < block_outline || tex_uv.x > 1 - block_outline || tex_uv.y < block_outline || tex_uv.y > 1 - block_outline)
                        albedo = float3(0.1, 0.4, 1.0) * 0.2 + luminance * 2;
                }
#endif
                color += albedo * light;
            } else {
                color += sample_sky(sun_ray, cam_ray.nrm);
            }
#endif
        }
    }

    // {
    //     RayIntersection temp_inter = ray_box_intersect(cam_ray, view_intersection_block_pos, view_intersection_block_pos + 1.0);
    //     overlay(color, depth, shaded(sun_ray, float3(1, 1, 1) * 0.5, temp_inter.nrm), temp_inter.dist, temp_inter.hit, 0.2);
    // }

    color *= inv_subsamples.x * inv_subsamples.y;

    draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 2 - 0, globals[0].frame_dim.y / 2 - 4, 1, 9);
    draw_rect(pixel_i.xy, color, globals[0].frame_dim.x / 2 - 4, globals[0].frame_dim.y / 2 - 0, 9, 1);

    // RayIntersection temp_inter;
    // for (int yi = 0; yi < 10; ++yi)
    //     for (int xi = 0; xi < 10; ++xi) {
    //         temp_inter = ray_sphere_intersect(cam_ray, float3(512 + xi * 2, 0, 512 + yi * 2), 1);
    //         draw(color, depth, getTexture2DArray<float4>(globals[0].texture_index).Load(int4((temp_inter.nrm.x * 0.5 + 0.5) * 16, (temp_inter.nrm.z * 0.5 + 0.5) * 16, int(GLOBALS_DEFINE.time * 5) % 30, 0)).rgb, temp_inter.dist, temp_inter.hit);
    //     }

    RWTexture2D<float4> output_image = daxa::getRWTexture2D<float4>(p.output_image_i);
    float4 prev_val = output_image[pixel_i.xy];
    float4 new_val = float4(pow(color, float3(1, 1, 1)), 1);
    // if (pixel_i.x > globals[0].frame_dim.x / 2) {
    //     output_image[pixel_i.xy] = prev_val * 0.99 + new_val * 0.01;
    // } else {
        output_image[pixel_i.xy] = new_val;
    // }
}
