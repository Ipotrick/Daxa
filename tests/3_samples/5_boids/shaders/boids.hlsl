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

// static const float PI = 3.141592f;
// static const float MAX_BOID_VIEW_DST = 0.05f;
// static const float BOIDS_DECELLERATION_DIST = MAX_BOID_VIEW_DST * 0.5f;
// static const float BOIDS_DECELLERATION = 0.02f;
// static const float MAX_BOID_VIEW_ANGLE = PI * 2.0f * 0.5f;
// static const float MAX_BOID_FOLLOW_ANGLE = MAX_BOID_VIEW_ANGLE * 0.25f;
// static const float MAX_BOID_FOLLOW_IGNORE_ANGLE = MAX_BOID_FOLLOW_ANGLE * 0.5f;
// static const float MAX_BOID_SPEED = 0.1;
// static const float MIN_BOID_SPEED = 0.01;
// static const float BOID_FOLLOW_ANGLE_CHANGE_PER_SECOND = PI * 2.0f * 0.4f;
// static const float BOID_FOLLOW_SPEED_CHANGE_PER_SECOND = 0.0f;
// static const float BOID_DEFLECT_ANGLE_CHANGE_PER_SECOND = PI * 2.0f * 0.8f;
// static const float BOID_DEFLECT_SPEED_CHANGE_PER_SECOND = 0.0f;
// static const float BOID_SPEEDUP_PER_SECOND = 0.01f;
// static const float BOID_SPEED_RANDOMNESS = 0.0f;
// static const float BOID_ANGLE_RANDOMNESS = 0.0f;

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
        float2 forward = normalize(boid_state.dir);
        const float scl = 0.01f;
        result.col = float3(1, 0.05, 0.5) * boid_state.speed;
        switch (boid_instance)
        {
        case 0: result.pos += forward.xy * float2(+1.0f, +1.0f) * scl, result.col = 1; break;
        case 1: result.pos += forward.xy * float2(-1.0f, -1.0f) * scl + forward.yx * float2(-1.0f, +1.0f) * scl * 0.5; break;
        case 2: result.pos += forward.xy * float2(-1.0f, -1.0f) * scl - forward.yx * float2(-1.0f, +1.0f) * scl * 0.5; break;
        }
        return result;
    }

    float2 eval_field(uint tid, float2 pos, float2 vel)
    {
        float2 v1 = 0, v2 = 0, v3 = 0;
        float2 v4 = float2(
            (pos.x < -0.9) - (pos.x > 0.9) * clamp(abs(pos.x) - 0.9, 0, 0.1) * 10,
            (pos.y < -0.9) - (pos.y > 0.9) * clamp(abs(pos.y) - 0.9, 0, 0.1) * 10);

        float2 center_of_mass = 0;
        float2 c = 0;
        float2 average_vel = 0;

        float center_n = 0;
        float avg_vel_n = 0;

        for (uint i = 0; i < MAX_BOIDS; ++i)
        {
            BoidState other_boid = boid_states[i];
            float2 del = other_boid.pos - pos;

            if (dot(del, del) < 0.5 * 0.5)
            {
                center_of_mass += other_boid.pos;
                center_n += 1.0;
            }

            if (i == tid)
                continue;

            if (dot(del, del) < 0.1 * 0.1)
            {
                average_vel += other_boid.dir * other_boid.speed;
                avg_vel_n += 1.0;
            }

            if (dot(del, del) < 0.1 * 0.1)
            {
                c = c - del;
            }
        }

        center_of_mass *= 1.0 / center_n;
        average_vel *= 1.0 / avg_vel_n;

        v1 = (pos - center_of_mass) * 0.00;
        v2 = c * 0.00;
        v3 = (average_vel - vel) * 0.00;
        v4 = v4 * 0.00;

        return v1 + v2 + v3 + v4;
    }
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(BoidsBuffer);