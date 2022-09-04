#version 450

#include <shared.inl>

DAXA_PUSH_CONSTANT(UpdateBoidsPushConstant)

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint invocation = gl_GlobalInvocationID.x;

    if (invocation > MAX_BOIDS)
    {
        return;
    }

    vec2 new_position =
        daxa_GetBuffer(BoidBuffer, daxa_push.old_boid_buffer_id).boids[invocation].position +
        daxa_GetBuffer(BoidBuffer, daxa_push.old_boid_buffer_id).boids[invocation].direction *
        daxa_GetBuffer(BoidBuffer, daxa_push.old_boid_buffer_id).boids[invocation].speed *
        daxa_push.delta_time;

    new_position = (new_position + vec2(1.0f)) * 0.5f;

    new_position = vec2(fract(new_position.x), fract(new_position.y));
    
    new_position = (new_position * 2.0f) - vec2(1.0f);

    daxa_GetBuffer(BoidBuffer, daxa_push.boid_buffer_id).boids[invocation].position = new_position;
}