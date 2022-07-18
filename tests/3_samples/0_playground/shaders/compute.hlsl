#include "daxa/daxa.hlsl"
struct Push
{
    daxa::ImageId image_id;
    uint2 frame_dim;
};
[[vk::push_constant]] const Push p;

// clang-format off
[numthreads(8, 8, 1)] void main(uint3 pixel_i : SV_DispatchThreadID)
// clang-format on
{
    RWTexture2D<float4> render_image = daxa::get_RWTexture2D<float4>(p.image_id);

    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    float2 uv = float2(pixel_i.xy) / float2(p.frame_dim.xy);
    uv = (uv - 0.5) * float2(float(p.frame_dim.x) / float(p.frame_dim.y), 1);

    float3 col = float3(0, 0, 0);

    if (uv.x > 0)
    {
        col = float3(1, 0, 1);
    }

    render_image[pixel_i.xy] = float4(col, 1.0);
}
