
int2 get_tile_i(float2 p) {
    int2 tile_i = int2(p / 1) * 1;
    if (p.x < 0)
        tile_i.x -= 1;
    if (p.y < 0)
        tile_i.y -= 1;
    return tile_i;
}

float2 pixelspace_to_worldspace(StructuredBuffer<Globals> globals, float2 p) {
    float2 uv = p / float2(globals[0].frame_dim) * 2 - 1;
    uv.x *= float(globals[0].frame_dim.x) / globals[0].frame_dim.y;
    return uv * 10;
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

void dda(StructuredBuffer<Globals> globals, float2 pixel_pos, inout float3 color, Ray ray, int max_steps) {
    float2 delta_dist = float2(                   //
        ray.nrm.x == 0 ? 1 : abs(ray.inv_nrm.x),  //
        ray.nrm.y == 0 ? 1 : abs(ray.inv_nrm.y));

    int2 tile_i = get_tile_i(ray.o);

    float2 initial_dist;
    int2 tile_step;

    dda_setup_axis(initial_dist.x, tile_step.x, tile_i.x, ray.o.x, ray.nrm.x, delta_dist.x);
    dda_setup_axis(initial_dist.y, tile_step.y, tile_i.y, ray.o.y, ray.nrm.y, delta_dist.y);

    int2 total_steps = int2(0, 0);
    int axis = 0;

    color_add(color, float3(0.1, 0.1, 0.1), square_dist(pixel_pos, tile_i, 0.875) < 0);
    color_add(color, float3(0.1, 0.1, 0.1), manhattan_dist(tile_i, get_tile_i(pixel_pos)) < max_steps);

    for (int i = 0; i < max_steps; ++i) {
        float2 total_dist = initial_dist + delta_dist * total_steps;

        if (total_dist.x < total_dist.y) {
            axis = 0;
        } else {
            axis = 1;
        }

        color_set(color, float3(0.9, 0.9, 0.9), circle_dist(pixel_pos, ray.o + total_dist[axis] * ray.nrm, 0.1) < 0);

        total_steps[axis] += 1;
    }
}

SHADERTOY_NUMTHREADS void main(uint3 pixel_i : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_id);
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

    color_set(color, float3(0.15, 0.15, 0.15), line_segment_dist(pixel_pos, ray.o, rmb_pos, 0.1) < 0);
    color_set(color, float3(0.50, 0.20, 0.50), circle_dist(pixel_pos, ray.o, 0.1) < 0);
    color_set(color, float3(0.25, 0.25, 0.25), circle_dist(pixel_pos, rmb_pos, 0.1) < 0);

    dda(globals, pixel_pos, color, ray, 6);

    output_image[pixel_i.xy] = float4(color, 1);
}
