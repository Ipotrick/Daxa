#include "daxa/daxa.hlsl"
struct Push
{
    daxa::ImageId image_id;
    uint2 frame_dim;
};
[[vk::push_constant]] const Push p;

[numthreads(8, 8, 1)] void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float4> render_image = daxa::get_RWTexture2D<float4>(p.image_id);

    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    float2 uv = float2(pixel_i.xy) / float2(p.frame_dim.xy);
    uv = (uv - 0.5) * float2(float(p.frame_dim.x) / float(p.frame_dim.y), 1);

    float2 z = uv * 3;
    float2 c = z;

    for (uint i = 0; i < 100; ++i) {
        float2 z_ = z;
        z.x = z_.x * z_.x - z_.y * z_.y;
        z.y = 2.0 * z_.x * z_.y;
        z += c;
    }

    float3 col = length(z);

    render_image[pixel_i.xy] = float4(col, 1.0);
}
