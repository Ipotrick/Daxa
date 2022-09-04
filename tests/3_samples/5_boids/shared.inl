#pragma once

#include <daxa/daxa.inl>

#define MAX_BOIDS 100000
#define BOID_SCALE 0.002f

struct DrawPushConstant
{
    BufferId boid_buffer_id;
};

struct UpdateBoidsPushConstant
{
    BufferId boid_buffer_id;
    BufferId old_boid_buffer_id;
    f32 delta_time;
};

struct Boid
{
    f32vec2 position;
    f32vec2 direction;
    f32 speed;
    u32 pad[3];
};

struct BoidBuffer
{
    Boid boids[MAX_BOIDS];
};
DAXA_REGISTER_STRUCT_GET_BUFFER(BoidBuffer)