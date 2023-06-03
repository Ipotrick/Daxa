#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(UpdateBoidsPushConstant, push)

// Based in pseudo code from: https://people.ece.cornell.edu/land/courses/ece4760/labs/s2021/Boids/Boids.html
void update_boid(inout Boid boid, in const Boid old_boid, in uint boid_index, BufferPtr(Boids) old_boids_buffer)
{
    boid = old_boid;

    vec2 vis_other_pos_average = vec2(0,0);
    vec2 vis_other_vel_average = vec2(0,0);
    float vis_other_count = 0;
    vec2 vis_other_super_close_delta = vec2(0,0);
    for (uint other_i = 0; other_i < MAX_BOIDS; ++other_i)
    {
        if (other_i == boid_index)
        {
            continue;
        }
        Boid other = deref(old_boids_buffer).boids[other_i];
        const vec2 pos_delta = boid.position - other.position;
        const float squared_distance = dot(pos_delta,pos_delta);
        if (squared_distance < BOID_PROTECTED_RANGE_SQUARED)
        {
            vis_other_super_close_delta += boid.position - other.position;
        }
        else if (squared_distance < BOID_VIEW_RANGE_SQUARED)
        {
            vis_other_pos_average += other.position;
            vis_other_vel_average += other.speed;
            vis_other_count += 1;
        }
    }
    if (vis_other_count > 0.0f)
    {
        vis_other_pos_average /= vis_other_count;
        vis_other_vel_average /= vis_other_count;
        boid.speed = boid.speed + (vis_other_pos_average - boid.position) * BOID_COHERENCE + (vis_other_vel_average - boid.speed) * BOID_ALIGNMENT;
    }
    boid.speed += BOID_SEPERATION * vis_other_super_close_delta;
    if (boid.position.x > FIELD_SIZE)
    {
        boid.speed.x -= BOID_WALL_REPULSION;
    }
    if (boid.position.x < 0)
    {
        boid.speed.x += BOID_WALL_REPULSION;
    }
    if (boid.position.y > FIELD_SIZE)
    {
        boid.speed.y -= BOID_WALL_REPULSION;
    }
    if (boid.position.y < 0)
    {
        boid.speed.y += BOID_WALL_REPULSION;
    }
    const float speed = max(0.01, length(boid.speed));
    if (speed < BOID_MIN_SPEED)
    {
        boid.speed = boid.speed / speed * BOID_MIN_SPEED;
    }
    if (speed > BOID_MAX_SPEED)
    {
        boid.speed = boid.speed / speed * BOID_MAX_SPEED;
    }
    boid.position = boid.position + boid.speed * DELTA_TIME;
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint invocation = gl_GlobalInvocationID.x;
    if (invocation >= MAX_BOIDS)
    {
        return;
    }
    update_boid(
        deref(push.boids_buffer).boids[invocation],
        deref(push.old_boids_buffer).boids[invocation],
        invocation,
        push.old_boids_buffer);
}