#include DAXA_SHADER_INCLUDE

#include "boids.hlsl"

struct Push
{
    BufferId prev_boids_buffer_id;
    BufferId boids_buffer_id;
    f32 delta_time;
};
[[vk::push_constant]] Push push;

[numthreads(64, 1, 1)] void main(u32 tid
                                 : SV_DISPATCHTHREADID)
{
    if (tid >= 100 * 100)
        return;

    StructuredBuffer<BoidsBuffer> prev_boids_buffer = daxa::get_StructuredBuffer<BoidsBuffer>(push.prev_boids_buffer_id);
    StructuredBuffer<BoidsBuffer> boids_buffer = daxa::get_StructuredBuffer<BoidsBuffer>(push.boids_buffer_id);

    BoidState me = prev_boids_buffer[0].boid_states[tid];

    f32vec2 vel = f32vec2(0, 0);

    vel += prev_boids_buffer[0].eval_field(tid, me.pos, vel);

    me.speed = length(vel);
    me.dir = vel / me.speed;
    me.speed = clamp(me.speed, 0.01, 0.1);

    vel = me.dir * me.speed;

    // me.pos += vel * push.delta_time;
    // me.pos = frac(me.pos * 0.5 + 0.5) * 2.0 - 1.0;

    u32 xi = tid % 100;
    u32 yi = tid / 100;

    me.pos = f32vec2(xi, yi) * 0.01 * 2.0 - 1.0;

    boids_buffer[0].boid_states[tid].pos = me.pos;
    // boids_buffer[0].boid_states[tid].dir = me.dir;
    // boids_buffer[0].boid_states[tid].speed = me.speed;
}
