#include "daxa/daxa.hlsl"

#include "boids.hlsl"

struct Push
{
    daxa::BufferId prev_boids_buffer_id;
    daxa::BufferId boids_buffer_id;
    float delta_time;
};
[[vk::push_constant]] Push push;

uint rand_hash(uint x) {
    x += (x << 10u);
    x ^= (x >> 6u);
    x += (x << 3u);
    x ^= (x >> 11u);
    x += (x << 15u);
    return x;
}
uint rand_hash(uint2 v) { return rand_hash(v.x ^ rand_hash(v.y)); }
uint rand_hash(uint3 v) {
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z));
}
uint rand_hash(uint4 v) {
    return rand_hash(v.x ^ rand_hash(v.y) ^ rand_hash(v.z) ^ rand_hash(v.w));
}
float rand_float_construct(uint m) {
    const uint ieee_mantissa = 0x007FFFFFu;
    const uint ieee_one = 0x3F800000u;
    m &= ieee_mantissa;
    m |= ieee_one;
    float f = asfloat(m);
    return f - 1.0;
}
float rand(float x) { return rand_float_construct(rand_hash(asuint(x))); }

[numthreads(64, 1, 1)]
void main(uint tid : SV_DISPATCHTHREADID)
{
    if (tid < MAX_BOIDS)
    {
        StructuredBuffer<BoidsBuffer> prev_boids_buffer = daxa::get_StructuredBuffer<BoidsBuffer>(push.prev_boids_buffer_id);
        StructuredBuffer<BoidsBuffer> boids_buffer = daxa::get_StructuredBuffer<BoidsBuffer>(push.boids_buffer_id);

        BoidState me = prev_boids_buffer[0].boid_states[tid];

        float acc_weight = 0.0f;
        float acc_dir_angle_change = 0.0f;
        float acc_speed_change = 0.0f;
        for (uint i = 0; i < MAX_BOIDS; ++i)
        {
            if (i == tid)
            {
                continue;
            }
            BoidState other_boid = prev_boids_buffer[0].boid_states[i];

            float2 me_to_other = normalize(other_boid.pos - me.pos);
            float distance = length(other_boid.pos - me.pos);
            float angle = acos(dot(me.dir, me_to_other));
            float2 dir_tangent = float2(me.dir.y, -me.dir.x);
            float tangent_sign = sign(dot(dir_tangent, me_to_other));
            bool looking_at_bum = dot(other_boid.dir, me.pos) < 0.0f;
            if (distance < MAX_BOID_VIEW_DST && angle < MAX_BOID_VIEW_ANGLE)
            {
                if (angle < MAX_BOID_FOLLOW_ANGLE && looking_at_bum)
                {
                    if (angle > MAX_BOID_FOLLOW_IGNORE_ANGLE)
                    {
                        // follow
                        float angle_change = BOID_FOLLOW_ANGLE_CHANGE_PER_SECOND * push.delta_time * -tangent_sign;
                        float speed_change = BOID_FOLLOW_SPEED_CHANGE_PER_SECOND * push.delta_time;
                        float weight = 0.8f;

                        acc_weight += weight;
                        acc_dir_angle_change += weight * angle_change;
                        acc_speed_change += weight * speed_change;
                    }
                }
                else{
                    // deflect
                    float angle_change = BOID_DEFLECT_ANGLE_CHANGE_PER_SECOND * push.delta_time * tangent_sign;
                    float speed_change = BOID_DEFLECT_SPEED_CHANGE_PER_SECOND * push.delta_time;
                    float weight = 0.2f;

                    acc_weight += weight;
                    acc_dir_angle_change += weight * angle_change;
                    acc_speed_change += weight * speed_change;
                }
            }
            if (distance < BOIDS_DECELLERATION_DIST)
            {
                acc_weight += 1.0f;
                acc_speed_change += BOIDS_DECELLERATION * push.delta_time;
            }
        }
        
        float dir_angle_change = 0;
        float speed_change = 0;

        dir_angle_change += (rand(push.delta_time + tid + 0.05) - 0.5) * push.delta_time * BOID_ANGLE_RANDOMNESS;
        speed_change += (rand(push.delta_time + tid + 0.04) - 0.5) * push.delta_time * BOID_SPEED_RANDOMNESS;

        if (acc_weight > 0.0f)
        {
            float rcp_weight = 1.0f / acc_weight;
            dir_angle_change += rcp_weight * acc_dir_angle_change;
            speed_change += rcp_weight * acc_speed_change;
        }

        speed_change += BOID_SPEEDUP_PER_SECOND * push.delta_time;

        float dir_angle = atan2(me.dir.y, me.dir.x);
        dir_angle += dir_angle_change;
        me.dir = float2(cos(dir_angle), sin(dir_angle));
        me.speed = clamp(me.speed + speed_change, MIN_BOID_SPEED, MAX_BOID_SPEED);

        me.pos = (me.pos + me.speed * push.delta_time * me.dir);

        // me.pos = frac(me.pos * 0.5f + float2(0.5f, 0.5f)) * 2.0f - float2(1.0f, 1.0f);

        me.pos = frac(me.pos * 0.5 + 0.5) * 2.0 - 1.0;

        boids_buffer[0].boid_states[tid] = me;
    }
}
