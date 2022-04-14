#include "utils/noise.hlsl"

#define CHUNK_SIZE 128

int2 get_tile_i(float2 p) {
    int2 tile_i = int2(p);
    if (p.x < 0)
        tile_i.x -= 1;
    if (p.y < 0)
        tile_i.y -= 1;
    return tile_i;
}

float2 pixelspace_to_worldspace(StructuredBuffer<Globals> globals, float2 p) {
    float2 uv = p / float2(globals[0].frame_dim) * 2 - 1;
    uv.x *= float(globals[0].frame_dim.x) / globals[0].frame_dim.y;
    return uv * CHUNK_SIZE / 2;
}

float grid_dist(float2 p, float s, float t) {
    // clang-format off
    return min(
        abs(frac(p.x * s - 0.5) - 0.5),
        abs(frac(p.y * s - 0.5) - 0.5)
    ) * 2 - t * s;
    // clang-format on
}

float square_dist(float2 p, float2 c, float r) {
    // clang-format off
    return max(
        abs(c.x - p.x + 0.5),
        abs(c.y - p.y + 0.5)
    ) * 2 - r;
    // clang-format on
}

float circle_dist(float2 p, float2 c, float r) { return length(p - c) - r; }

float line_segment_dist(float2 p, float2 a, float2 b, float r) {
    float2 ba = b - a;
    float2 pa = p - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
    return length(pa - h * ba) - r;
}

int manhattan_dist(int2 a, int2 b) {
    return abs(a.x - b.x) + abs(a.y - b.y);  //
}

void color_set(inout float3 color, float3 new_color, bool should) {
    if (should)
        color = new_color;
}
void color_add(inout float3 color, float3 new_color, bool should) {
    if (should)
        color += new_color;
}
void color_mix(inout float3 color, float3 new_color, float fac) {
    color = color * (1 - fac) + new_color * fac;  //
}

struct Ray {
    float2 o;
    float2 nrm;
    float2 inv_nrm;
};

void dda_setup_axis(inout float initial_dist, inout int tile_step, in int tile_i, in float o, in float nrm,
                    in float delta_dist) {
    if (nrm < 0) {
        tile_step = -1;
        initial_dist = (o - tile_i) * delta_dist;
    } else {
        tile_step = 1;
        initial_dist = (tile_i + 1 - o) * delta_dist;
    }
}

bool is_tile_occluding(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    tile_i = tile_i + CHUNK_SIZE / 2;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE)
        return false;
    return buf0[0].data[tile_i.x + tile_i.y * CHUNK_SIZE];
}
bool is_tile_x2(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    tile_i = (tile_i + CHUNK_SIZE / 2) / 2;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE / 2 || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE / 2)
        return false;
    return buf0[0].data_1[(tile_i.x + tile_i.y * CHUNK_SIZE / 2)];
}
bool is_tile_x4(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    tile_i = (tile_i + CHUNK_SIZE / 2) / 4;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE / 4 || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE / 4)
        return false;
    return buf0[0].data_2[(tile_i.x + tile_i.y * CHUNK_SIZE / 4)];
}
bool is_tile_x8(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    tile_i = (tile_i + CHUNK_SIZE / 2) / 8;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE / 8 || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE / 8)
        return false;
    return buf0[0].data_3[(tile_i.x + tile_i.y * CHUNK_SIZE / 8)];
}
bool is_tile_x16(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    tile_i = (tile_i + CHUNK_SIZE / 2) / 16;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE / 16 || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE / 16)
        return false;
    return buf0[0].data_4[(tile_i.x + tile_i.y * CHUNK_SIZE / 16)];
}
bool is_tile_x32(in StructuredBuffer<Buf0> buf0, in int2 tile_i) {
    tile_i = (tile_i + CHUNK_SIZE / 2) / 32;
    if (tile_i.x < 0 || tile_i.x >= CHUNK_SIZE / 32 || tile_i.y < 0 || tile_i.y >= CHUNK_SIZE / 32)
        return false;
    return buf0[0].data_5[(tile_i.x + tile_i.y * CHUNK_SIZE / 32)];
}

void dda(StructuredBuffer<Globals> globals, float2 pixel_pos, inout float3 color, in StructuredBuffer<Buf0> buf0,
         Ray ray, int max_steps) {
    float2 delta_dist = float2(                           //
        ray.nrm.x == 0 ? max_steps : abs(ray.inv_nrm.x),  //
        ray.nrm.y == 0 ? max_steps : abs(ray.inv_nrm.y));

    int2 tile_i = get_tile_i(ray.o);
    int2 pixel_tile_i = get_tile_i(pixel_pos);

    float2 initial_dist;
    int2 tile_step;

    dda_setup_axis(initial_dist.x, tile_step.x, tile_i.x, ray.o.x, ray.nrm.x, delta_dist.x);
    dda_setup_axis(initial_dist.y, tile_step.y, tile_i.y, ray.o.y, ray.nrm.y, delta_dist.y);

    int2 total_axis_steps = int2(0, 0);
    int axis = 0;

    color_set(color, float3(0.1, 0.2, 0.5), is_tile_occluding(buf0, pixel_tile_i));
    // color_add(color, float3(0.1, 0.2, 0.5), is_tile_x8(buf0, pixel_tile_i));

    for (int i = 0; i < max_steps; ++i) {
        int2 current_tile_i = tile_i + tile_step * total_axis_steps;
        if (is_tile_occluding(buf0, current_tile_i)) {
            color_set(color, float3(0.6, 0.1, 0.1), square_dist(pixel_pos, current_tile_i, 0.875) < 0);
            break;
        }
        color_add(color, float3(0.1, 0.1, 0.1), square_dist(pixel_pos, current_tile_i, 0.875) < 0);

        float2 total_dist = initial_dist + delta_dist * total_axis_steps;

        if (total_dist.x < total_dist.y) {
            axis = 0;
        } else {
            axis = 1;
        }

        float2 hit_p = ray.o + total_dist[axis] * ray.nrm;
        color_set(color, float3(0.9, 0.9, 0.9), circle_dist(pixel_pos, hit_p, 0.1) < 0);

        total_axis_steps[axis] += 1;
    }
}

SHADERTOY_NUMTHREADS_MAIN void main(uint3 pixel_i : SV_DispatchThreadID) {
    if (pixel_i.x == 0 && pixel_i.y == 0) {
        printf("get rekt");
    }

    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_id);
    StructuredBuffer<Buf0> buf0 = getBuffer<Buf0>(p.buf0_id);
    RWTexture2D<float4> output_image = getRWTexture2D<float4>(p.output_image_id);

    float2 mouse_pos = pixelspace_to_worldspace(globals, globals[0].mouse_pos);
    float2 lmb_pos = pixelspace_to_worldspace(globals, globals[0].lmb_pos);
    float2 rmb_pos = pixelspace_to_worldspace(globals, globals[0].rmb_pos);

    float2 pixel_pos = pixelspace_to_worldspace(globals, pixel_i.xy);
    float3 color = float3(0.05, 0.05, 0.05);

    color_set(color, float3(0.2, 0.2, 0.2), grid_dist(pixel_pos, 1, 0.0625) < 0);

    Ray ray;
    ray.o = lmb_pos;
    ray.nrm = normalize(rmb_pos - ray.o);
    ray.inv_nrm = 1 / ray.nrm;

    color_set(color, float3(0.15, 0.15, 0.15), line_segment_dist(pixel_pos, ray.o, rmb_pos, 0.5) < 0);
    color_set(color, float3(0.50, 0.20, 0.50), circle_dist(pixel_pos, ray.o, 0.5) < 0);
    color_set(color, float3(0.25, 0.25, 0.25), circle_dist(pixel_pos, rmb_pos, 0.5) < 0);

    dda(globals, pixel_pos, color, buf0, ray, CHUNK_SIZE * 2);

    output_image[pixel_i.xy] = float4(color, 1);
}

SHADERTOY_NUMTHREADS_BUF0 void buf0_main(uint3 tile_i : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_id);
    StructuredBuffer<Buf0> buf0 = getBuffer<Buf0>(p.buf0_id);

    int i = tile_i.x + tile_i.y * CHUNK_SIZE;

    switch (p.output_image_id) {
    case 0: {
        FractalNoiseConfig noise_conf = {
            /* .amplitude   = */ 1.0f,
            /* .persistance = */ 0.4f,
            /* .scale       = */ 0.1f,
            /* .lacunarity  = */ 2,
            /* .octaves     = */ 6,
        };
        buf0[0].data[i] = fractal_noise(float3(tile_i.xy, 0), noise_conf) - 0.5 < 0;
    } break;
    case 1: {
        if (tile_i.x % 2 == 0 && tile_i.y % 2 == 0) {
            i = tile_i.x / 2 + tile_i.y / 2 * CHUNK_SIZE / 2;
            if (i < CHUNK_SIZE * CHUNK_SIZE / 2)
                buf0[0].data_1[i] =  //
                    buf0[0].data[tile_i.x + 0 + (tile_i.y + 0) * CHUNK_SIZE] ||
                    buf0[0].data[tile_i.x + 1 + (tile_i.y + 0) * CHUNK_SIZE] ||
                    buf0[0].data[tile_i.x + 0 + (tile_i.y + 1) * CHUNK_SIZE] ||
                    buf0[0].data[tile_i.x + 1 + (tile_i.y + 1) * CHUNK_SIZE];
        }
    } break;
    case 2: {
        if (tile_i.x % 4 == 0 && tile_i.y % 4 == 0) {
            i = tile_i.x / 4 + tile_i.y / 4 * CHUNK_SIZE / 4;
            tile_i = tile_i / 2;
            if (i < CHUNK_SIZE * CHUNK_SIZE / 4)
                buf0[0].data_2[i] =  //
                    buf0[0].data_1[tile_i.x + 0 + (tile_i.y + 0) * CHUNK_SIZE / 2] ||
                    buf0[0].data_1[tile_i.x + 1 + (tile_i.y + 0) * CHUNK_SIZE / 2] ||
                    buf0[0].data_1[tile_i.x + 0 + (tile_i.y + 1) * CHUNK_SIZE / 2] ||
                    buf0[0].data_1[tile_i.x + 1 + (tile_i.y + 1) * CHUNK_SIZE / 2];
        }
    } break;
    case 3: {
        if (tile_i.x % 8 == 0 && tile_i.y % 8 == 0) {
            i = tile_i.x / 8 + tile_i.y / 8 * CHUNK_SIZE / 8;
            tile_i = tile_i / 4;
            if (i < CHUNK_SIZE * CHUNK_SIZE / 8)
                buf0[0].data_3[i] =  //
                    buf0[0].data_2[tile_i.x + 0 + (tile_i.y + 0) * CHUNK_SIZE / 4] ||
                    buf0[0].data_2[tile_i.x + 1 + (tile_i.y + 0) * CHUNK_SIZE / 4] ||
                    buf0[0].data_2[tile_i.x + 0 + (tile_i.y + 1) * CHUNK_SIZE / 4] ||
                    buf0[0].data_2[tile_i.x + 1 + (tile_i.y + 1) * CHUNK_SIZE / 4];
        }
    } break;
    case 4: {
        if (tile_i.x % 16 == 0 && tile_i.y % 16 == 0) {
            i = tile_i.x / 16 + tile_i.y / 16 * CHUNK_SIZE / 16;
            tile_i = tile_i / 8;
            if (i < CHUNK_SIZE * CHUNK_SIZE / 16)
                buf0[0].data_4[i] =  //
                    buf0[0].data_3[tile_i.x + 0 + (tile_i.y + 0) * CHUNK_SIZE / 8] ||
                    buf0[0].data_3[tile_i.x + 1 + (tile_i.y + 0) * CHUNK_SIZE / 8] ||
                    buf0[0].data_3[tile_i.x + 0 + (tile_i.y + 1) * CHUNK_SIZE / 8] ||
                    buf0[0].data_3[tile_i.x + 1 + (tile_i.y + 1) * CHUNK_SIZE / 8];
        }
    } break;
    case 5: {
        if (tile_i.x % 32 == 0 && tile_i.y % 32 == 0) {
            i = tile_i.x / 32 + tile_i.y / 32 * CHUNK_SIZE / 32;
            tile_i = tile_i / 16;
            if (i < CHUNK_SIZE * CHUNK_SIZE / 32)
                buf0[0].data_5[i] =  //
                    buf0[0].data_4[tile_i.x + 0 + (tile_i.y + 0) * CHUNK_SIZE / 16] ||
                    buf0[0].data_4[tile_i.x + 1 + (tile_i.y + 0) * CHUNK_SIZE / 16] ||
                    buf0[0].data_4[tile_i.x + 0 + (tile_i.y + 1) * CHUNK_SIZE / 16] ||
                    buf0[0].data_4[tile_i.x + 1 + (tile_i.y + 1) * CHUNK_SIZE / 16];
        }
    } break;
    }
}
