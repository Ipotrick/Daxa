
SHADERTOY_NUMTHREADS void main(uint3 pixel_i : SV_DispatchThreadID) {
    StructuredBuffer<Globals> globals = getBuffer<Globals>(p.globals_id);
    RWTexture2D<float4> output_image = getRWTexture2D<float4>(p.output_image_id);

    float2 uv = float2(pixel_i.xy) / float2(globals[0].frame_dim);

    float3 color = 0.5 + 0.5 * cos(globals[0].time + uv.xyx + float3(0, 2, 4));

    output_image[pixel_i.xy] = float4(color, 1);
}
