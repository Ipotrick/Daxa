#version 450

#include <shared.inl>

DAXA_PUSH_CONSTANT(DrawPushConstant)

void main()
{
    uint boid_index = gl_VertexIndex / 3;

    vec2 corner_position;
    switch (gl_VertexIndex % 3)
    {
        case 0: corner_position = vec2(-1.0f, -1.0f); break;
        case 1: corner_position = vec2( 1.0f, -1.0f); break;
        case 2: corner_position = vec2( 0.0f,  1.0f); break;
    }
    corner_position *= BOID_SCALE;

    corner_position += daxa_GetBuffer(BoidBuffer, daxa_push.boid_buffer_id).boids[boid_index].position;

    gl_Position = vec4(corner_position, 0.0f, 1.0f);
}