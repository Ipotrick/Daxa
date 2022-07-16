#include <daxa/daxa.hlsl>

struct Push
{
    daxa::ImageId image_id;
    uint2 frame_dim;
};

[[vk::push_constant]] const Push p;

[numthreads(8, 8, 1)] void main(uint3 pixel_i: SV_DispatchThreadID)
{
    RWTexture2D<float4> render_image = p.image_id.as_RWTexture2D<float4>();

    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    render_image[pixel_i] = float4(float2(pixel_i.xy) / float2(p.frame_dim), 0.5, 1.0);
}
