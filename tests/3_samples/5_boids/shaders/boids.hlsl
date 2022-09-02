#pragma once

static const u32 MAX_BOIDS = 1000;

struct BoidState
{
    f32vec2 pos;
    f32vec2 dir;
    f32 speed;
    u32 _pad0;
};

struct BoidVertex
{
    f32vec2 pos;
    f32vec3 col;
};

// static const f32 PI = 3.141592f;
// static const f32 MAX_BOID_VIEW_DST = 0.05f;
// static const f32 BOIDS_DECELLERATION_DIST = MAX_BOID_VIEW_DST * 0.5f;
// static const f32 BOIDS_DECELLERATION = 0.02f;
// static const f32 MAX_BOID_VIEW_ANGLE = PI * 2.0f * 0.5f;
// static const f32 MAX_BOID_FOLLOW_ANGLE = MAX_BOID_VIEW_ANGLE * 0.25f;
// static const f32 MAX_BOID_FOLLOW_IGNORE_ANGLE = MAX_BOID_FOLLOW_ANGLE * 0.5f;
// static const f32 MAX_BOID_SPEED = 0.1;
// static const f32 MIN_BOID_SPEED = 0.01;
// static const f32 BOID_FOLLOW_ANGLE_CHANGE_PER_SECOND = PI * 2.0f * 0.4f;
// static const f32 BOID_FOLLOW_SPEED_CHANGE_PER_SECOND = 0.0f;
// static const f32 BOID_DEFLECT_ANGLE_CHANGE_PER_SECOND = PI * 2.0f * 0.8f;
// static const f32 BOID_DEFLECT_SPEED_CHANGE_PER_SECOND = 0.0f;
// static const f32 BOID_SPEEDUP_PER_SECOND = 0.01f;
// static const f32 BOID_SPEED_RANDOMNESS = 0.0f;
// static const f32 BOID_ANGLE_RANDOMNESS = 0.0f;

struct BoidsBuffer
{
    BoidState boid_states[MAX_BOIDS];

    BoidVertex get_vertex(u32 vert_i)
    {
        u32 boid_index = vert_i / 3;
        u32 boid_instance = vert_i - boid_index * 3;
        BoidState boid_state = boid_states[boid_index];
        BoidVertex result;
        result.pos = boid_state.pos;
        f32vec2 forward = normalize(boid_state.dir);
        const f32 scl = 0.01f;
        result.col = f32vec3(1, 0.05, 0.5) * boid_state.speed;
        switch (boid_instance)
        {
        case 0: result.pos += forward.xy * f32vec2(+1.0f, +1.0f) * scl, result.col = 1; break;
        case 1: result.pos += forward.xy * f32vec2(-1.0f, -1.0f) * scl + forward.yx * f32vec2(-1.0f, +1.0f) * scl * 0.5; break;
        case 2: result.pos += forward.xy * f32vec2(-1.0f, -1.0f) * scl - forward.yx * f32vec2(-1.0f, +1.0f) * scl * 0.5; break;
        }
        return result;
    }

    f32vec2 eval_field(u32 tid, f32vec2 pos, f32vec2 vel)
    {
        f32vec2 v1 = 0, v2 = 0, v3 = 0;
        f32vec2 v4 = f32vec2(
            (pos.x < -0.9) - (pos.x > 0.9) * clamp(abs(pos.x) - 0.9, 0, 0.1) * 10,
            (pos.y < -0.9) - (pos.y > 0.9) * clamp(abs(pos.y) - 0.9, 0, 0.1) * 10);

        f32vec2 center_of_mass = 0;
        f32vec2 c = 0;
        f32vec2 average_vel = 0;

        f32 center_n = 0;
        f32 avg_vel_n = 0;

        for (u32 i = 0; i < MAX_BOIDS; ++i)
        {
            BoidState other_boid = boid_states[i];
            f32vec2 del = other_boid.pos - pos;

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

        v1 = (pos - center_of_mass) * 0.1;
        v2 = c * 0.01;
        v3 = (average_vel - vel) * 0.01;
        v4 = v4 * 0.01;

        return v1 + v2 + v3 + v4;
    }
};

DAXA_DEFINE_GET_STRUCTURED_BUFFER(BoidsBuffer);