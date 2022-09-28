#version 450

#include <shared.inl>

DAXA_USE_PUSH_CONSTANT(MipmappingComputePushConstant)

f32 segment_distance(f32vec2 p, f32vec2 a, f32vec2 b)
{
    f32vec2 ba = b - a;
    f32vec2 pa = p - a;
    f32 h = clamp(dot(pa, ba) / dot(ba, ba), 0, 1);
    return length(pa - h * ba);
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    u32vec3 pixel_i = gl_GlobalInvocationID.xyz;
    if (pixel_i.x >= push_constant.frame_dim.x || pixel_i.y >= push_constant.frame_dim.y)
        return;

    f32vec2 render_size = push_constant.frame_dim;
    f32vec2 inv_render_size = f32vec2(1.0) / render_size;
    f32vec2 pixel_pos = pixel_i.xy;
    f32vec2 mouse_pos = f32vec2(push_constant.compute_input.mouse_x, push_constant.compute_input.mouse_y);
    f32vec2 prev_mouse_pos = f32vec2(push_constant.compute_input.p_mouse_x, push_constant.compute_input.p_mouse_y);

    f32vec2 uv = pixel_pos * inv_render_size;

    f32 dist = segment_distance(pixel_pos, prev_mouse_pos, mouse_pos);

    if (dist < push_constant.compute_input.paint_radius)
    {
        f32vec3 col = push_constant.compute_input.paint_col;

        imageStore(
            daxa_GetRWImage(image2D, rgba32f, push_constant.image_id),
            i32vec2(pixel_i.xy),
            f32vec4(col, 1));
    }
}
