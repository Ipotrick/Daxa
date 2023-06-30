#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1
#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(MipmappingComputePushConstant, push)

f32 segment_distance(f32vec2 p, f32vec2 a, f32vec2 b)
{
    f32vec2 ba = b - a;
    f32vec2 pa = p - a;
    f32 h = clamp(dot(pa, ba) / dot(ba, ba), 0, 1);
    return length(pa - h * ba);
}

#define INPUT deref(push.gpu_input)

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    u32vec3 pixel_i = gl_GlobalInvocationID.xyz;
    if (pixel_i.x >= push.frame_dim.x || pixel_i.y >= push.frame_dim.y)
        return;

    f32vec2 render_size = push.frame_dim;
    f32vec2 inv_render_size = f32vec2(1.0) / render_size;
    f32vec2 pixel_pos = pixel_i.xy;
    f32vec2 mouse_pos = f32vec2(INPUT.mouse_x, INPUT.mouse_y);
    f32vec2 prev_mouse_pos = f32vec2(INPUT.p_mouse_x, INPUT.p_mouse_y);

    f32vec2 uv = pixel_pos * inv_render_size;

    f32 dist = segment_distance(pixel_pos, prev_mouse_pos, mouse_pos);

    if (dist < INPUT.paint_radius)
    {
        f32vec3 col = INPUT.paint_col;
        imageStore(daxa_image2D(push.image), i32vec2(pixel_i.xy), f32vec4(col, 1));
    }
}
