
#include "drawing/common.hlsl"
#include "utils/intersect.hlsl"
#include "utils/noise.hlsl"
#include "chunk.hlsl"

uint tile_texture_index(GLOBALS_PARAM BlockID block_id, BlockFace face) {
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
    case BlockID::Lava:            return 13 + int(GLOBALS_DEFINE.time * 6) % 8;
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

void draw_rect(in uint2 pixel_i, inout float3 color, int px, int py, int sx, int sy) {
    if (pixel_i.x >= px &&
        pixel_i.x < px + sx &&
        pixel_i.y >= py &&
        pixel_i.y < py + sy)
        color = float3(1, 1, 1);
}

void draw(inout float3 color, inout float depth, in float3 new_color, in float new_depth, bool should) {
    if (should && new_depth < depth) {
        color = new_color;
        depth = new_depth;
    }
}

float3 shaded(in Ray sun_ray, in float3 col, in float3 nrm) {
    float3 light = sun_col * max(dot(nrm, sun_ray.nrm), 0) + sample_sky(sun_ray, nrm) * 0.6;
    return col * light;
}

void overlay(inout float3 color, inout float depth, in float3 new_color, in float new_depth, bool should, float fac) {
    if (should) {
        color = color * (1 - fac) + new_color * fac;
    }
}

float3 ortho(float3 v) {
    return lerp(float3(-v.y, v.x, 0.0), float3(0.0, -v.z, v.y), step(abs(v.x), abs(v.z)));
}

float3 around(float3 v, float3 z) {
    float3 t = ortho(z), b = cross(z, t);
    return mad(t, float3(v.x, v.x, v.x), mad(b, float3(v.y, v.y, v.y), z * v.z));
}

float3 isotropic(float rp, float c) {
    // sin(a) = sqrt(1.0 - cos(a)^2) , in the interval [0, PI/2] relevant for us
    float p = 2 * 3.14159 * rp, s = sqrt(1.0 - c * c);
    return float3(cos(p) * s, sin(p) * s, c);
}

float3 rand_pt(float3 n, float2 rnd) {
    float c = sqrt(rnd.y);
    return around(isotropic(rnd.x, c), n);
}

[numthreads(8, 8, 1)] void main(uint3 pixel_i
                                : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_sb);

    if (pixel_i.x >= GLOBALS_DEFINE.frame_dim.x ||
        pixel_i.y >= GLOBALS_DEFINE.frame_dim.y)
        return;

    float3 front = mul(GLOBALS_DEFINE.viewproj_mat, float4(0, 0, 1, 0)).xyz;
    float3 right = mul(GLOBALS_DEFINE.viewproj_mat, float4(1, 0, 0, 0)).xyz;
    float3 up = mul(GLOBALS_DEFINE.viewproj_mat, float4(0, 1, 0, 0)).xyz;

    float3 view_intersection_pos = GLOBALS_DEFINE.pick_pos[0].xyz;
    Ray cam_ray;
    cam_ray.o = GLOBALS_DEFINE.pos.xyz;

    Ray sun_ray;
    float sun_angle = 0.3;
    float sun_yz = -abs(cos(sun_angle)) * 2;
    sun_ray.nrm = normalize(float3(sin(sun_angle) * 3, sun_yz, -0.2 * sun_yz));
    sun_ray.inv_nrm = 1 / sun_ray.nrm;

    const float2 subsamples = float2(SUBSAMPLE_N, SUBSAMPLE_N);
    const float2 inv_subsamples = 1 / subsamples;
    float2 inv_frame_dim = 1 / float2(GLOBALS_DEFINE.frame_dim);
    float aspect = float(GLOBALS_DEFINE.frame_dim.x) * inv_frame_dim.y;
    int2 i_uv = int2(pixel_i.xy);
    float uv_rand_offset = GLOBALS_DEFINE.time;
    float3 color = float3(0, 0, 0);
    float depth = 100000;

    float2 uv_offset =
#if JITTER_VIEW
        float2(rand(float2(i_uv + uv_rand_offset + 10)),
               rand(float2(i_uv + uv_rand_offset)));
#else
        float2(0, 0);
#endif
    float2 uv = (float2(i_uv) + uv_offset * inv_subsamples) * inv_frame_dim * 2 - 1;

    for (uint yi = 0; yi < subsamples.y; ++yi) {
        for (uint xi = 0; xi < subsamples.x; ++xi) {
            float2 view_uv = (uv + inv_frame_dim * float2(xi, yi) * inv_subsamples) * GLOBALS_DEFINE.fov * float2(aspect, 1);

            cam_ray.nrm = normalize(front + view_uv.x * right + view_uv.y * up);
            cam_ray.inv_nrm = 1 / cam_ray.nrm;

            RayIntersection ray_chunk_intersection = trace_chunks(GLOBALS_ARG cam_ray);
#if VISUALIZE_STEP_COMPLEXITY
            color.r += log(float(ray_chunk_intersection.steps) * 1 / MAX_STEPS + 1);
#else
            float3 intersection_pos = get_intersection_pos_corrected(cam_ray, ray_chunk_intersection);
            BlockID block_id = load_block_id(GLOBALS_ARG intersection_pos);
            int3 intersection_block_pos = int3(intersection_pos);
            int3 view_intersection_block_pos = int3(view_intersection_pos);

            // if (ray_chunk_intersection.hit && block_id == BlockID::Water) {
            //     cam_ray.o = intersection_pos + ray_chunk_intersection.nrm * 0.01;
            //     cam_ray.nrm = normalize(reflect(cam_ray.nrm, ray_chunk_intersection.nrm));
            //     cam_ray.inv_nrm = 1 / cam_ray.nrm;
            //     ray_chunk_intersection = trace_chunks(cam_ray);
            //     intersection_pos = get_intersection_pos(cam_ray, ray_chunk_intersection);
            //     block_id = get_block_id(intersection_pos);
            // }
            // if (ray_chunk_intersection.hit && block_id == BlockID::Air) {
            //     uint tile = load_tile(intersection_pos);
            //     float sdf_value = float((tile & SDF_DIST_MASK) >> 0x18) / 128;
            //     color += float3(sdf_value);
            // } else
            if (ray_chunk_intersection.hit) {
#if ENABLE_SHADOWS
                sun_ray.o = intersection_pos + ray_chunk_intersection.nrm * 0.002;
                RayIntersection sun_ray_chunk_intersection = trace_chunks(GLOBALS_ARG sun_ray);
                float val = float(!sun_ray_chunk_intersection.hit);
                val = max(val * dot(ray_chunk_intersection.nrm, sun_ray.nrm), 0.01);
                float3 light = val * sun_col + sample_sky(sun_ray, ray_chunk_intersection.nrm) * 0.2;
#else
                // float3 light = shaded(sun_ray, float3(1, 1, 1), ray_chunk_intersection.nrm);
                float3 light = float3(1, 1, 1);
#endif
                float3 b_uv = float3(int3(intersection_pos) % 16) / 16;
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

                uint tex_id = tile_texture_index(GLOBALS_ARG block_id, face_id);

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
                // float3 albedo = getTexture2DArray<float4>(GLOBALS_DEFINE.texture_index).Sample(
                //     getSampler<void>(GLOBALS_DEFINE.sampler_index),
                //     float3(tex_uv.x, tex_uv.y, float(tex_id)),
                //     int2(0, 0),
                //     1.0f).rgb;
                float3 albedo = getTexture2DArray<float4>(GLOBALS_DEFINE.texture_index).Load(int4(tex_uv.x * 16, tex_uv.y * 16, tex_id, 0)).rgb;

                // float occlusion = 0;
                // float3 nrm;
                // for (uint i = 0; i < 16; ++i) {
                //     nrm = normalize(rand_pt(ray_chunk_intersection.nrm, rand_vec2(tex_uv.xy + pixel_i.xy + i + GLOBALS_DEFINE.time)));
                //     Ray bounce_ray;
                //     bounce_ray.o = intersection_pos + ray_chunk_intersection.nrm * 0.002;
                //     bounce_ray.nrm = nrm;
                //     bounce_ray.inv_nrm = 1 / bounce_ray.nrm;
                //     RayIntersection bounce_ray_intersection = ray_step_voxels(GLOBALS_ARG bounce_ray, bounce_ray.o - float3(20, 20, 20), bounce_ray.o + float3(20, 20, 20), 10);
                //     if (bounce_ray_intersection.hit)
                //         occlusion += 0.0625;
                // }
                // occlusion = 1 - occlusion;
                // light *= occlusion * occlusion;

                // float3 albedo = (nrm);
#elif ALBEDO == ALBEDO_DEBUG_POS
                float3 albedo = b_uv;
#elif ALBEDO == ALBEDO_DEBUG_NRM
                float3 albedo = ray_chunk_intersection.nrm * 0.5 + 0.5;
#elif ALBEDO == ALBEDO_DEBUG_DIST
                float d = ray_chunk_intersection.dist;
                float3 albedo = float3(d, d, d) * 0.01;
#elif ALBEDO == ALBEDO_DEBUG_RANDOM
                float3 albedo = float3(rand(int3(intersection_pos)), rand(int3(intersection_pos + 10)), rand(int3(intersection_pos + 20)));
                // float3 albedo = float3(block_id) / 32;
                // if (block_id == BlockID::Air)
                //     albedo = float3(1, 0, 1);
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

#if SHOW_SINGLE_RAY
    Ray ray;
    ray.o = GLOBALS_DEFINE.single_ray_pos.xyz;
    ray.nrm = GLOBALS_DEFINE.single_ray_nrm.xyz;
    ray.inv_nrm = 1 / ray.nrm;

    RayIntersection temp_inter;

    float3 side_cols[3] = {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1)};

    {
        float3 b_min = float3(0, 0, 0), b_max = float3(CHUNK_NX, CHUNK_NY, CHUNK_NZ) * CHUNK_SIZE;

        RayIntersection result;
        result.hit = false;
        result.dist = 0;
        result.nrm = float3(0, 0, 0);
        result.steps = 0;

        uint max_steps = GLOBALS_DEFINE.single_ray_steps;

        DDA_RunState run_state;
        run_state.outside_bounds = false;
        run_state.side = 0;

        int3 tile_i = int3(ray.o);

        // temp_inter = ray_wirebox_intersect(cam_ray, float3(tile_i), float3(tile_i + 1));
        // draw(color, depth, shaded(sun_ray, float3(1), temp_inter.nrm), temp_inter.dist, temp_inter.hit);

        // dda_start
        DDA_StartResult dda_start;

        dda_start.delta_dist = float3(
            ray.nrm.x == 0 ? 1 : abs(ray.inv_nrm.x),
            ray.nrm.y == 0 ? 1 : abs(ray.inv_nrm.y),
            ray.nrm.z == 0 ? 1 : abs(ray.inv_nrm.z));
        run_state.tile_i = int3(ray.o / 1) * 1;
        run_state.tile_i_x4 = int3(ray.o / 4) * 4;
        run_state.tile_i_x16 = int3(ray.o / 16) * 16;
        if (ray.nrm.x < 0) {
            dda_start.ray_step.x = -1;
            run_state.to_side_dist.x = (ray.o.x - run_state.tile_i.x) * dda_start.delta_dist.x;
            run_state.to_side_dist_x4.x = (ray.o.x - run_state.tile_i_x4.x) * dda_start.delta_dist.x;
            run_state.to_side_dist_x16.x = (ray.o.x - run_state.tile_i_x16.x) * dda_start.delta_dist.x;
        } else {
            dda_start.ray_step.x = 1;
            run_state.to_side_dist.x = (run_state.tile_i.x + 1 - ray.o.x) * dda_start.delta_dist.x;
            run_state.to_side_dist_x4.x = (run_state.tile_i_x4.x + 4 - ray.o.x) * dda_start.delta_dist.x;
            run_state.to_side_dist_x16.x = (run_state.tile_i_x16.x + 16 - ray.o.x) * dda_start.delta_dist.x;
        }
        if (ray.nrm.y < 0) {
            dda_start.ray_step.y = -1;
            run_state.to_side_dist.y = (ray.o.y - run_state.tile_i.y) * dda_start.delta_dist.y;
            run_state.to_side_dist_x4.y = (ray.o.y - run_state.tile_i_x4.y) * dda_start.delta_dist.y;
            run_state.to_side_dist_x16.y = (ray.o.y - run_state.tile_i_x16.y) * dda_start.delta_dist.y;
        } else {
            dda_start.ray_step.y = 1;
            run_state.to_side_dist.y = (run_state.tile_i.y + 1 - ray.o.y) * dda_start.delta_dist.y;
            run_state.to_side_dist_x4.y = (run_state.tile_i_x4.y + 4 - ray.o.y) * dda_start.delta_dist.y;
            run_state.to_side_dist_x16.y = (run_state.tile_i_x16.y + 16 - ray.o.y) * dda_start.delta_dist.y;
        }
        if (ray.nrm.z < 0) {
            dda_start.ray_step.z = -1;
            run_state.to_side_dist.z = (ray.o.z - run_state.tile_i.z) * dda_start.delta_dist.z;
            run_state.to_side_dist_x4.z = (ray.o.z - run_state.tile_i_x4.z) * dda_start.delta_dist.z;
            run_state.to_side_dist_x16.z = (ray.o.z - run_state.tile_i_x16.z) * dda_start.delta_dist.z;
        } else {
            dda_start.ray_step.z = 1;
            run_state.to_side_dist.z = (run_state.tile_i.z + 1 - ray.o.z) * dda_start.delta_dist.z;
            run_state.to_side_dist_x4.z = (run_state.tile_i_x4.z + 4 - ray.o.z) * dda_start.delta_dist.z;
            run_state.to_side_dist_x16.z = (run_state.tile_i_x16.z + 16 - ray.o.z) * dda_start.delta_dist.z;
        }
        dda_start.initial_to_side_dist = run_state.to_side_dist;
        dda_start.initial_to_side_dist_x4 = run_state.to_side_dist_x4;
        dda_start.initial_to_side_dist_x16 = run_state.to_side_dist_x16;

        // end dda_start

        // dda_main

        run_state.hit = false;
        uint x1_steps = 0;

        for (run_state.total_steps = 0; run_state.total_steps < max_steps; ++run_state.total_steps) {
            if (run_state.hit)
                break;
            if (x1_steps >= max_steps)
                break;
            // #if ENABLE_X16
            // temp_inter = ray_wirebox_intersect(cam_ray, float3(run_state.tile_i_x16), float3(run_state.tile_i_x16 + 16), 0.2);
            // draw(color, depth, shaded(sun_ray, side_cols[run_state.side], temp_inter.nrm), temp_inter.dist, temp_inter.hit);
            temp_inter = ray_box_intersect(cam_ray, float3(run_state.tile_i_x16), float3(run_state.tile_i_x16 + 16));
            overlay(color, depth, shaded(sun_ray, side_cols[run_state.side], temp_inter.nrm), temp_inter.dist, temp_inter.hit, 0.1);

            x1_steps++;
            run_dda_step<16>(run_state.to_side_dist_x16, run_state.tile_i_x16, run_state.side, dda_start);

            if (x_load_presence<16>(GLOBALS_ARG run_state.tile_i_x16)) {
                RayIntersection x16_intersection;
                x16_intersection.nrm = float3(dda_start.ray_step * 1);
                switch (run_state.side) {
                case 0: x16_intersection.dist = run_state.to_side_dist_x16.x - dda_start.delta_dist.x * 16; break;
                case 1: x16_intersection.dist = run_state.to_side_dist_x16.y - dda_start.delta_dist.y * 16; break;
                case 2: x16_intersection.dist = run_state.to_side_dist_x16.z - dda_start.delta_dist.z * 16; break;
                }

                // temp_inter = ray_sphere_intersect(cam_ray, get_intersection_pos(ray, x16_intersection), 0.1);
                // draw(color, depth, shaded(sun_ray, float3(1), temp_inter.nrm), temp_inter.dist, temp_inter.hit);

                run_state.tile_i_x4 = int3(get_intersection_pos_corrected(ray, x16_intersection));
                run_state.tile_i_x4 = (run_state.tile_i_x4 / 4) * 4;
                run_state.to_side_dist_x4 = abs(float3(int3(ray.o / 4) - run_state.tile_i_x4 / 4)) * dda_start.delta_dist * 4 + dda_start.initial_to_side_dist_x4;

                // temp_inter = ray_wirebox_intersect(cam_ray, float3(run_state.tile_i_x4), float3(run_state.tile_i_x4 + 4), 0.1);
                // draw(color, depth, shaded(sun_ray, float3(1), temp_inter.nrm), temp_inter.dist, temp_inter.hit);

                for (uint i = 0; i < 12; ++i) {
                    if (x1_steps >= max_steps)
                        break;

                    // temp_inter = ray_wirebox_intersect(cam_ray, float3(run_state.tile_i_x4), float3(run_state.tile_i_x4 + 4), 0.1);
                    // draw(color, depth, shaded(sun_ray, side_cols[run_state.side], temp_inter.nrm), temp_inter.dist, temp_inter.hit);
                    temp_inter = ray_box_intersect(cam_ray, float3(run_state.tile_i_x4), float3(run_state.tile_i_x4 + 4));
                    overlay(color, depth, shaded(sun_ray, side_cols[run_state.side], temp_inter.nrm), temp_inter.dist, temp_inter.hit, 0.2);
                    // #endif
                    x1_steps++;
                    run_dda_step<4>(run_state.to_side_dist_x4, run_state.tile_i_x4, run_state.side, dda_start);

                    if (x_load_presence<4>(GLOBALS_ARG run_state.tile_i_x4)) {
                        RayIntersection x4_intersection;
                        x4_intersection.nrm = float3(dda_start.ray_step * 1);
                        switch (run_state.side) {
                        case 0: x4_intersection.dist = run_state.to_side_dist_x4.x - dda_start.delta_dist.x * 4; break;
                        case 1: x4_intersection.dist = run_state.to_side_dist_x4.y - dda_start.delta_dist.y * 4; break;
                        case 2: x4_intersection.dist = run_state.to_side_dist_x4.z - dda_start.delta_dist.z * 4; break;
                        }

                        // temp_inter = ray_sphere_intersect(cam_ray, get_intersection_pos(ray, x4_intersection), 0.1);
                        // draw(color, depth, shaded(sun_ray, float3(1), temp_inter.nrm), temp_inter.dist, temp_inter.hit);

                        run_state.tile_i = int3(get_intersection_pos_corrected(ray, x4_intersection));
                        run_state.to_side_dist = abs(float3(int3(ray.o) - run_state.tile_i)) * dda_start.delta_dist + dda_start.initial_to_side_dist;

                        // temp_inter = ray_wirebox_intersect(cam_ray, float3(run_state.tile_i), float3(run_state.tile_i + 1), 0.1);
                        // draw(color, depth, shaded(sun_ray, float3(1), temp_inter.nrm), temp_inter.dist, temp_inter.hit);

                        for (uint j = 0; j < 12; ++j) {
                            if (x1_steps >= max_steps)
                                break;

                            // temp_inter = ray_wirebox_intersect(cam_ray, float3(run_state.tile_i), float3(run_state.tile_i + 1), 0.05);
                            // draw(color, depth, shaded(sun_ray, side_cols[run_state.side], temp_inter.nrm), temp_inter.dist, temp_inter.hit);
                            temp_inter = ray_box_intersect(cam_ray, float3(run_state.tile_i), float3(run_state.tile_i + 1));
                            overlay(color, depth, shaded(sun_ray, side_cols[run_state.side], temp_inter.nrm), temp_inter.dist, temp_inter.hit, 0.4);

                            uint tile = load_tile(run_state.tile_i);
                            if (is_block_occluding(get_block_id(tile))) {
                                run_state.hit = true;
                                break;
                            }

                            x1_steps++;
                            run_dda_step<1>(run_state.to_side_dist, run_state.tile_i, run_state.side, dda_start);

                            if (run_state.tile_i / 4 == run_state.tile_i_x4)
                                break;
                            if (!point_box_contains(run_state.tile_i, b_min, b_max)) {
                                run_state.outside_bounds = true;
                                break;
                            }
                        }
                    }
                    if (run_state.hit || run_state.tile_i_x4 / 4 == run_state.tile_i_x16)
                        break;
                    if (!point_box_contains(run_state.tile_i_x4, b_min, b_max)) {
                        run_state.outside_bounds = true;
                        break;
                    }
                    // #if ENABLE_X16
                }
            }
            if (!point_box_contains(run_state.tile_i_x16, b_min, b_max)) {
                run_state.outside_bounds = true;
                break;
            }
            // #endif
        }
        run_state.total_steps = x1_steps;

        // end dda_main

        result.hit = run_state.hit;

        float x = run_state.to_side_dist.x;
        float y = run_state.to_side_dist.y;
        float z = run_state.to_side_dist.z;

        float dx = dda_start.delta_dist.x;
        float dy = dda_start.delta_dist.y;
        float dz = dda_start.delta_dist.z;

        if (run_state.outside_bounds) {
            result.dist += 1.0f;
            result.hit = false;
        }

        switch (run_state.side) {
        case 0: result.dist += x - dx, result.nrm = float3(-dda_start.ray_step.x, 0, 0); break;
        case 1: result.dist += y - dy, result.nrm = float3(0, -dda_start.ray_step.y, 0); break;
        case 2: result.dist += z - dz, result.nrm = float3(0, 0, -dda_start.ray_step.z); break;
        }

        if (result.hit) {
            temp_inter = ray_cylinder_intersect(cam_ray, ray.o, ray.o + ray.nrm * result.dist, 0.04);
            draw(color, depth, shaded(sun_ray, side_cols[run_state.side], temp_inter.nrm), temp_inter.dist, temp_inter.hit);
        }

        temp_inter = ray_sphere_intersect(cam_ray, ray.o, 0.05);
        draw(color, depth, shaded(sun_ray, float3(1, 1, 1), temp_inter.nrm), temp_inter.dist, temp_inter.hit);

        temp_inter = ray_sphere_intersect(cam_ray, ray.o + ray.nrm * result.dist, 0.05);
        draw(color, depth, shaded(sun_ray, float3(1, 1, 1), temp_inter.nrm), temp_inter.dist, temp_inter.hit);
    }

#endif

    color *= inv_subsamples.x * inv_subsamples.y;

    draw_rect(pixel_i.xy, color, GLOBALS_DEFINE.frame_dim.x / 2 - 0, GLOBALS_DEFINE.frame_dim.y / 2 - 4, 1, 9);
    draw_rect(pixel_i.xy, color, GLOBALS_DEFINE.frame_dim.x / 2 - 4, GLOBALS_DEFINE.frame_dim.y / 2 - 0, 9, 1);

    // RayIntersection temp_inter;
    // for (int yi = 0; yi < 10; ++yi)
    //     for (int xi = 0; xi < 10; ++xi) {
    //         temp_inter = ray_sphere_intersect(cam_ray, float3(512 + xi * 2, 0, 512 + yi * 2), 1);
    //         draw(color, depth, getTexture2DArray<float4>(GLOBALS_DEFINE.texture_index).Load(int4((temp_inter.nrm.x * 0.5 + 0.5) * 16, (temp_inter.nrm.z * 0.5 + 0.5) * 16, int(GLOBALS_DEFINE.time * 5) % 30, 0)).rgb, temp_inter.dist, temp_inter.hit);
    //     }

    RWTexture2D<float4> output_image = getRWTexture2D<float4>(p.output_image_i);
    output_image[pixel_i.xy] = float4(pow(color, float3(1, 1, 1)), 1);
}
