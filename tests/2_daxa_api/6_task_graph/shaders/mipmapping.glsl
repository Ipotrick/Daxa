#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(MipmappingComputePushConstant, push)

float segment_distance(vec2 p, vec2 a, vec2 b)
{
    vec2 ba = b - a;
    vec2 pa = p - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0, 1);
    return length(pa - h * ba);
}

#define INPUT deref(push.gpu_input)

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    uvec3 pixel_i = gl_GlobalInvocationID.xyz;
    if (pixel_i.x >= push.frame_dim.x || pixel_i.y >= push.frame_dim.y)
        return;

    vec2 render_size = push.frame_dim;
    vec2 inv_render_size = vec2(1.0) / render_size;
    vec2 pixel_pos = pixel_i.xy;
    vec2 mouse_pos = vec2(INPUT.mouse_x, INPUT.mouse_y);
    vec2 prev_mouse_pos = vec2(INPUT.p_mouse_x, INPUT.p_mouse_y);

    vec2 uv = pixel_pos * inv_render_size;

    float dist = segment_distance(pixel_pos, prev_mouse_pos, mouse_pos);

    if (dist < INPUT.paint_radius)
    {
        vec3 col = INPUT.paint_col;
        imageStore(daxa_image2D(push.image), ivec2(pixel_i.xy), vec4(col, 1));
    }
}
