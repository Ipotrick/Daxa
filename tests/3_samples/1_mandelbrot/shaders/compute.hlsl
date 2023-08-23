#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <daxa/daxa.inl>
#include <shared.inl>

#include "custom file!!"

[[vk::push_constant]] const ComputePush p;

#define CENTER f32vec2(-0.694008, -0.324998)
#define SUBSAMPLES 2

f32vec3 hsv2rgb(f32vec3 c)
{
    f32vec4 k = f32vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    f32vec3 p = abs(frac(c.xxx + k.xyz) * 6.0 - k.www);
    return c.z * lerp(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

f32vec3 mandelbrot_colored(f32vec2 pixel_p)
{
    f32vec2 uv = pixel_p / f32vec2(p.frame_dim.xy);
    uv = (uv - 0.5) * f32vec2(f32(p.frame_dim.x) / f32(p.frame_dim.y), 1);
    StructuredBuffer<GpuInput> input = p.input_buffer_id.get<StructuredBuffer<GpuInput> >();
    f32 time = input[0].time;
    f32 scale = 12.0 / (exp(time) + 0.0001);
    f32vec2 z = uv * scale * 2 + CENTER;
    f32vec2 c = z;
    u32 i = 0;
    for (; i < 512; ++i)
    {
        f32vec2 z_ = z;
        z.x = z_.x * z_.x - z_.y * z_.y;
        z.y = 2.0 * z_.x * z_.y;
        z += c;
        if (dot(z, z) > 256 * 256)
            break;
    }
    f32vec3 col = 0;
    if (i != 512)
    {
        f32 l = i;
        f32 sl = l - log2(log2(dot(z, z))) + 4.0;
        sl = pow(sl * 0.01, 1.0);
        col = hsv2rgb(f32vec3(sl, 1, 1));
#if MY_TOGGLE
        col = 1 - col;
#endif
    }
    return col;
}

// clang-format off
[numthreads(8, 8, 1)]
void main(u32vec3 pixel_i : SV_DispatchThreadID)
// clang-format on
{
    //RWTexture2D<f32vec4> render_image = daxa::get_RWTexture2D<f32vec4>(p.image_id); // old
    RWTexture2D<float4> render_image = p.image_id.get<RWTexture2D<float4> >();
    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;
    f32vec3 col = 0;
    for (i32 yi = 0; yi < SUBSAMPLES; ++yi)
    {
        for (i32 xi = 0; xi < SUBSAMPLES; ++xi)
        {
            f32vec2 offset = f32vec2(xi, yi) / f32(SUBSAMPLES);
            col += mandelbrot_colored(f32vec2(pixel_i.xy) + offset);
        }
    }
    col *= 1.0 / f32(SUBSAMPLES * SUBSAMPLES);
    render_image[pixel_i.xy] = f32vec4(col, 1);
}
