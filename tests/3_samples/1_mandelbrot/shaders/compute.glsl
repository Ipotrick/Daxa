#version 450

#include <shared.inl>

DAXA_PUSH_CONSTANT(ComputePush)

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    u32vec3 pixel_i = gl_GlobalInvocationID.xyz;
    if (pixel_i.x >= daxa_push.frame_dim.x || pixel_i.y >= daxa_push.frame_dim.y)
        return;
    imageStore(
        daxa_GetRWImage(image2D, rgba32f, daxa_push.image_id), 
        i32vec2(pixel_i.xy), 
        f32vec4(1, 0, 1, 1)
    );
}
