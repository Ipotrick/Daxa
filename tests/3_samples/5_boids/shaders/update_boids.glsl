#define DAXA_ENABLE_SHADER_NO_NAMESPACE
#include <shared.inl>

DAXA_USE_PUSH_CONSTANT(UpdateBoidsPushConstant)

float good_sign(float v)
{
    return v > 0.0f ? 1.0f : -1.0f;
}

float angle_between_normals(vec2 a, vec2 b)
{
    return acos(dot(a, b));
}

float angle_between_normals_sign(vec2 a, vec2 b)
{
    vec2 tangent_direction = vec2(-a.y, a.x);
    return good_sign(dot(tangent_direction, b));
}

float signed_angle_between_normals(vec2 a, vec2 b)
{
    return angle_between_normals_sign(a, b) * angle_between_normals(a, b);
}

void boid_update_position(inout Boid boid, in Boid old_boid, vec2 new_boid_direction)
{
    vec2 new_position = old_boid.position + new_boid_direction * BOID_SPEED * SIMULATION_DELTA_TIME_S;

    new_position = clamp(new_position, vec2(0.001f), vec2(FIELD_SIZE - 0.001f));

    boid.position = new_position;
    boid.direction = new_boid_direction;
}

void boid_avoid_walls(inout Boid old_boid, inout float steer_angle, inout float acc_steering_weight)
{
    vec2 wall_normal = vec2(0.0f);
    float wall_dist = FIELD_SIZE;
    // left wall
    if (old_boid.position.x < BOID_VIEW_RANGE)
    {
        wall_normal += vec2(1.0f, 0.0f);
        wall_dist = min(wall_dist, old_boid.position.x);
    }
    // top wall
    if (old_boid.position.y < BOID_VIEW_RANGE)
    {
        wall_normal += vec2(0.0f, 1.0f);
        wall_dist = min(wall_dist, old_boid.position.y);
    }
    // right wall
    if (old_boid.position.x + BOID_VIEW_RANGE > FIELD_SIZE)
    {
        wall_normal += vec2(-1.0f, 0.0f);
        wall_dist = min(wall_dist, FIELD_SIZE - old_boid.position.x);
    }
    // bottom wall
    if (old_boid.position.y + BOID_VIEW_RANGE > FIELD_SIZE)
    {
        wall_normal += vec2(0.0f, -1.0f);
        wall_dist = min(wall_dist, FIELD_SIZE - old_boid.position.y);
    }

    if (wall_normal != vec2(0.0f))
    {
        wall_normal = normalize(wall_normal);
        vec2 boid_to_wall_dir = -wall_normal;
        float relative_closeness = 1.0f - (wall_dist / float(BOID_VIEW_RANGE)) * 0.5f;
        float angle_to_move_boid_away = signed_angle_between_normals(old_boid.direction, boid_to_wall_dir);
        angle_to_move_boid_away = clamp(angle_to_move_boid_away, -BOID_STEER_PER_TICK * 2, BOID_STEER_PER_TICK * 2);
        float acc_weight = relative_closeness * 2;
        steer_angle += angle_to_move_boid_away * relative_closeness * acc_weight;
        acc_steering_weight += acc_weight;
    }
}

void update_boid(inout Boid boid, in Boid old_boid, in uint boid_index, BufferRef(Boids) old_boids_buffer)
{
    float acc_steer_angle = 0.0f;
    float acc_steer_angle_weight = 0.0f;
    boid_avoid_walls(old_boid, acc_steer_angle, acc_steer_angle_weight);

    for (uint i = 0; i < 0; ++i)
    {
        if (i == boid_index)
            continue;

        Boid other = old_boids_buffer.boids[i];

        float dst_to_other = length(other.position - old_boid.position);
        float closeness_to_other = 1.0f - (dst_to_other / float(BOID_VIEW_RANGE));
        vec2 dir_to_other = normalize(other.position - old_boid.position);
        float signed_angle_of_boid_dir_to_other_dir = signed_angle_between_normals(old_boid.direction, dir_to_other);

        if (dst_to_other < BOID_VIEW_RANGE || abs(signed_angle_of_boid_dir_to_other_dir) > BOID_VIEW_ANGLE)
            continue;

        // avoid other boids:
        float avoid_acc_weight = closeness_to_other;
        float avoid_angle_add = clamp(signed_angle_of_boid_dir_to_other_dir, -BOID_STEER_PER_TICK, BOID_STEER_PER_TICK) * BOIDS_AVOID_FACTOR;

        // gabe take a look:
        // acc_steer_angle += avoid_acc_weight * closeness_to_other * avoid_angle_add;
        // acc_steer_angle_weight += avoid_acc_weight;
    }

    vec2 new_boid_direction = boid.direction;
    if (acc_steer_angle_weight > 0.0f)
    {
        acc_steer_angle /= acc_steer_angle_weight;
        new_boid_direction = vec2(
            dot(old_boid.direction, vec2(cos(acc_steer_angle), sin(acc_steer_angle))),
            dot(old_boid.direction, vec2(-sin(acc_steer_angle), cos(acc_steer_angle))));
    }
    boid_update_position(boid, old_boid, new_boid_direction);
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
        push_constant.boids_buffer.boids[invocation],
        push_constant.old_boids_buffer.boids[invocation],
        invocation,
        push_constant.old_boids_buffer);
}