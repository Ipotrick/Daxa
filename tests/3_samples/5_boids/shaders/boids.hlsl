#pragma once

static const uint MAX_BOIDS = 1000;

struct BoidState
{
    float2 pos;
    float2 dir;
    float speed;
    uint _pad0;
};

struct BoidVertex
{
    float2 pos;
    float3 col;
};

static const float PI = 3.141592f;
static const float MAX_BOID_VIEW_DST = 0.05f;
static const float BOIDS_DECELLERATION_DIST = MAX_BOID_VIEW_DST * 0.5f;
static const float BOIDS_DECELLERATION = 0.02f;
static const float MAX_BOID_VIEW_ANGLE = PI * 2.0f * 0.5f;
static const float MAX_BOID_FOLLOW_ANGLE = MAX_BOID_VIEW_ANGLE * 0.25f;
static const float MAX_BOID_FOLLOW_IGNORE_ANGLE = MAX_BOID_FOLLOW_ANGLE * 0.5f;
static const float MAX_BOID_SPEED = 0.1;
static const float MIN_BOID_SPEED = 0.01;
static const float BOID_FOLLOW_ANGLE_CHANGE_PER_SECOND = PI * 2.0f * 0.4f;
static const float BOID_FOLLOW_SPEED_CHANGE_PER_SECOND = 0.0f;
static const float BOID_DEFLECT_ANGLE_CHANGE_PER_SECOND = PI * 2.0f * 0.8f;
static const float BOID_DEFLECT_SPEED_CHANGE_PER_SECOND = 0.0f;
static const float BOID_SPEEDUP_PER_SECOND = 0.01f;
static const float BOID_SPEED_RANDOMNESS = 0.1f;
static const float BOID_ANGLE_RANDOMNESS = 1.0f;

struct BoidsBuffer
{
    BoidState boid_states[MAX_BOIDS];

    BoidVertex get_vertex(uint vert_i)
    {
        uint boid_index = vert_i / 3;
        uint boid_instance = vert_i - boid_index * 3;
        BoidState boid_state = boid_states[boid_index];
        BoidVertex result;
        result.pos = boid_state.pos;
        float2 forward = boid_state.dir;
        const float scl = 0.01f;
        switch (boid_instance)
        {
        case 0: result.pos += forward.xy * float2(+1.0f, +1.0f) * scl; break;
        case 1: result.pos -= forward.xy * float2(+1.0f, +1.0f) * scl + forward.yx * float2(-1.0f, +0.5f) * scl; break;
        case 2: result.pos -= forward.xy * float2(+1.0f, +1.0f) * scl - forward.yx * float2(-1.0f, +0.5f) * scl; break;
        }
        float3 color = float3(1, 0.1, 0.1) * boid_state.speed;
        color = max(color, float3(0.01, 0.01, 0.01));
        color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
        result.col = color;
        return result;
    }
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(BoidsBuffer);