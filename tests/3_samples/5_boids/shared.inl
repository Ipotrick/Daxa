#pragma once

#include <daxa/daxa.inl>

#define MAX_BOIDS 10000
#define FIELD_SIZE 100
#define BOID_SCALE 0.2f
#define BOID_VIEW_ANGLE (0.65f * 3.14f)
#define BOID_VIEW_RANGE 5
#define BOID_BLIND_RANGE 0.1f
#define BOID_STEER_PER_SECOND 0.25
#define SIMULATION_DELTA_TIME_MS 5
#define SIMULATION_DELTA_TIME_S (float(SIMULATION_DELTA_TIME_MS) * 0.001f)
#define BOID_STEER_PER_TICK (BOID_STEER_PER_SECOND * SIMULATION_DELTA_TIME_S)
#define BOID_SPEED 7.0f

#define BOIDS_SEPERATION_FACTOR 1.0f
#define BOIDS_COHESION_FACTOR 0.01f
#define BOIDS_CENTER_FACTOR 1.0f

struct Boid
{
    f32vec2 position;
    f32vec2 direction;
};

DAXA_DECL_BUFFER_STRUCT(
    Boids, 
    {
        Boid boids[MAX_BOIDS];
    }
);

struct DrawPushConstant
{
    BufferRef(Boids) boids_buffer;
    f32vec2 axis_scaling;
};

struct UpdateBoidsPushConstant
{
    BufferRef(Boids) old_boids_buffer;
    BufferId boids_buffer_id;
};
