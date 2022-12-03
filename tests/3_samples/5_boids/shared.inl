#pragma once

#include <daxa/daxa.inl>

#define MAX_BOIDS 10000
#define FIELD_SIZE 100
#define BOID_SCALE 0.2f
#define BOID_VIEW_ANGLE (0.65f * 3.14f)
#define BOID_VIEW_RANGE 5
#define BOID_BLIND_RANGE 0.1f
#define BOID_STEER_PER_SECOND 1.85f
#define SIMULATION_DELTA_TIME_MS 0.5f
#define SIMULATION_DELTA_TIME_S (SIMULATION_DELTA_TIME_MS * 0.001f)
#define BOID_STEER_PER_TICK (BOID_STEER_PER_SECOND * SIMULATION_DELTA_TIME_S)
#define BOID_SPEED 7.0f

#define BOIDS_AVOID_FACTOR 1.0f
#define BOIDS_COHESION_FACTOR 0.01f
#define BOIDS_CENTER_FACTOR 1.0f

struct Boid
{
    daxa_f32vec2 position;
    daxa_f32vec2 direction;
};

struct Boids
{
    Boid boids[MAX_BOIDS];
};
DAXA_ENABLE_BUFFER_PTR(Boids)

struct GpuOutput
{
    daxa_f32 data;
};
DAXA_ENABLE_BUFFER_PTR(GpuOutput)

struct DrawPushConstant
{
    daxa_BufferPtr(Boids) boids_buffer;
    daxa_f32vec2 axis_scaling;
};

struct UpdateBoidsPushConstant
{
    daxa_RWBufferPtr(Boids) boids_buffer;
    daxa_BufferPtr(Boids) old_boids_buffer;
};
