#include <shared.inl>

#include <custom file!!>

DAXA_DECL_PUSH_CONSTANT(ComputePush, p)

#define CENTER vec2(-0.694008, -0.324998)
#define SUBSAMPLES 2

vec3 hsv2rgb(vec3 c)
{
    vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
    return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

vec3 mandelbrot_colored(vec2 pixel_p)
{
    vec2 uv = pixel_p / vec2(p.frame_dim.xy);
    uv = (uv - 0.5) * vec2(daxa_f32(p.frame_dim.x) / daxa_f32(p.frame_dim.y), 1);
    daxa_BufferPtr(GpuInput) gpu_input_ptr = daxa_BufferPtr(GpuInput)(daxa_id_to_address(p.input_buffer_id));
    daxa_f32 time = deref(gpu_input_ptr).time;
    daxa_f32 scale = 12.0 / (exp(time) + 0.0001);
    vec2 z = uv * scale * 2 + CENTER;
    vec2 c = z;
    daxa_u32 i = 0;
    for (; i < 512; ++i)
    {
        vec2 z_ = z;
        z.x = z_.x * z_.x - z_.y * z_.y;
        z.y = 2.0 * z_.x * z_.y;
        z += c;
        if (dot(z, z) > 256 * 256)
            break;
    }
    vec3 col = vec3(0);
    if (i != 512)
    {
        daxa_f32 l = i;
        daxa_f32 sl = l - log2(log2(dot(z, z))) + 4.0;
        sl = pow(sl * 0.01, 1.0);
        col = hsv2rgb(vec3(sl, 1, 1));
#if MY_TOGGLE
        col = 1 - col;
#endif
    }
    return col;
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    uvec3 pixel_i = gl_GlobalInvocationID.xyz;
    if (pixel_i.x >= p.frame_dim.x || pixel_i.y >= p.frame_dim.y)
        return;

    vec3 col = vec3(0, 0, 0);
    for (daxa_i32 yi = 0; yi < SUBSAMPLES; ++yi)
    {
        for (daxa_i32 xi = 0; xi < SUBSAMPLES; ++xi)
        {
            vec2 offset = vec2(xi, yi) / daxa_f32(SUBSAMPLES);
            col += mandelbrot_colored(vec2(pixel_i.xy) + offset);
        }
    }
    col *= 1.0 / daxa_f32(SUBSAMPLES * SUBSAMPLES);

    imageStore(daxa_image2D(p.image_id), ivec2(pixel_i.xy), vec4(col, 1));
}
