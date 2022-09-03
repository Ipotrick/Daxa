#include <shared.inl>

[[vk::push_constant]] const ComputePush p;

// clang-format off
[numthreads(8, 8, 1)]
void main(u32vec3 pixel_i : SV_DispatchThreadID)
// clang-format on
{
    RWTexture2D<f32vec4> render_image = daxa::get_RWTexture2D<f32vec4>(p.image_id);
    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;
    render_image[pixel_i.xy] = f32vec4(1, 0, 1, 1);
}
