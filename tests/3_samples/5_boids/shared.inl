#pragma once

#include <daxa/daxa.inl>

#define PI 3.14f

#define DELTA_TIME 0.01

#define MAX_BOIDS 10000
#define FIELD_SIZE 100
#define BOID_SCALE 0.25f
#define BOID_VIEW_RANGE 0.9f
#define BOID_VIEW_RANGE_SQUARED BOID_VIEW_RANGE*BOID_VIEW_RANGE
#define BOID_PROTECTED_RANGE 0.5f
#define BOID_PROTECTED_RANGE_SQUARED BOID_PROTECTED_RANGE*BOID_PROTECTED_RANGE
#define BOID_MIN_SPEED 10.f
#define BOID_MAX_SPEED 40.f

#define BOID_COHERENCE 1.00f
#define BOID_SEPERATION 20.00f
#define BOID_ALIGNMENT 1.00f
#define BOID_WALL_REPULSION 1.0f

struct Boid
{
    daxa_f32vec2 position;
    daxa_f32vec2 speed;
};

struct Boids
{
    Boid boids[MAX_BOIDS];
};
DAXA_DECL_BUFFER_PTR(Boids)

struct GpuOutput
{
    daxa_f32 data;
};
DAXA_DECL_BUFFER_PTR(GpuOutput)

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
