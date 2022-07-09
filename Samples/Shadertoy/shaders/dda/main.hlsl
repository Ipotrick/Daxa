// #include "dda.hlsl"
#include "dda_copy.hlsl"
// #include "world.hlsl"
// #include "drawing.hlsl"

SHADERTOY_NUMTHREADS_MAIN void main(uint3 pixel_i : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_id);
    RWTexture2D<float4> output_image = getRWTexture2D<float4>(p.output_image_id);
    StructuredBuffer<Buf0> buf0 = getBuffer<Buf0>(p.buf0_id);

    float2 mouse_pos = pixelspace_to_worldspace(globals, globals[0].mouse_pos);
    float2 lmb_pos = pixelspace_to_worldspace(globals, globals[0].lmb_pos);
    float2 rmb_pos = pixelspace_to_worldspace(globals, globals[0].rmb_pos);

    float2 pixel_pos = pixelspace_to_worldspace(globals, pixel_i.xy);
    float3 color = float3(0.0, 0.0, 0.0);

    int2 pixel_tile_i = get_tile_i<1>(pixel_pos);
    color_add(color, get_color<5>() * 0.1, check_presence<32>(buf0, pixel_tile_i));
    color_add(color, get_color<4>() * 0.1, check_presence<16>(buf0, pixel_tile_i));
    color_add(color, get_color<3>() * 0.1, check_presence<8>(buf0, pixel_tile_i));
    color_add(color, get_color<2>() * 0.1, check_presence<4>(buf0, pixel_tile_i));
    color_add(color, get_color<1>() * 0.1, check_presence<2>(buf0, pixel_tile_i));
    color_set(color, get_color<0>() * 1.0, check_presence<1>(buf0, pixel_tile_i));
    color_set(color, get_color<0>() * 1.0, check_presence<1>(buf0, pixel_tile_i));

    Ray ray;
    ray.o = lmb_pos;
    ray.nrm = normalize(rmb_pos - ray.o);
    ray.inv_nrm = 1 / ray.nrm;

    color_set(color, float3(0.15, 0.15, 0.15), line_segment_dist(pixel_pos, ray.o, rmb_pos, 0.125) < 0);
    color_set(color, float3(0.50, 0.20, 0.50), circle_dist(pixel_pos, ray.o, 0.125) < 0);
    color_set(color, float3(0.25, 0.25, 0.25), circle_dist(pixel_pos, rmb_pos, 0.125) < 0);

    dda(buf0, pixel_pos, color, ray, CHUNK_SIZE * 2);
    // dda(buf0, pixel_pos, color, ray, CHUNK_SIZE * 2);

    output_image[pixel_i.xy] = float4(color, 1);
}
