#version 450

#include <shared.inl>

DAXA_USE_PUSH_CONSTANT(UpdateBoidsPushConstant)

void update_boid_position(inout Boid boid, in Boid old_boid)
{
    vec2 new_position = old_boid.position + old_boid.direction * BOID_SPEED * SIMULATION_DELTA_TIME_S;

    new_position = clamp(new_position, vec2(0.0f), vec2(FIELD_SIZE));

    boid.position = new_position;
}

void update_boid(inout Boid boid, in Boid old_boid, in uint boid_index, BoidsBufferRef old_boids_buffer)
{
    float steer_angle = 0.0f;
    uint neighboring_boids = 0;

    for (uint i = 0; i < MAX_BOIDS; ++i)
    {
        if (boid_index == i)
        {
            continue;
        }

        const Boid other_boid = old_boids_buffer.boids[i];

        float to_other_distance = length(other_boid.position - boid.position);
        vec2 to_other_direction = normalize(other_boid.position - boid.position);

        bool other_boid_in_range = to_other_distance < BOID_VIEW_RANGE;
        bool other_boid_in_view_angle = acos(dot(to_other_direction, boid.direction)) < BOID_VIEW_ANGLE;

        vec2 tangent_direction = vec2(-boid.direction.y, boid.direction.x);

        float angle_between_directions = 
            acos(dot(boid.direction, other_boid.direction)) *
            sign(dot(tangent_direction, other_boid.position));

        if (other_boid_in_range && other_boid_in_view_angle)
        {
            neighboring_boids += 1;
            float relative_closeness = float(BOID_VIEW_RANGE) / to_other_distance;

            float seperation_steer = 
                BOIDS_SEPERATION_FACTOR *
                relative_closeness *
                float(BOID_STEER_PER_SECOND) * 
                float(SIMULATION_DELTA_TIME_S) *
                sign(dot(tangent_direction, to_other_direction));

            steer_angle += seperation_steer;
        }
    }

    if (neighboring_boids > 0)
    {
        float average_steer = clamp(steer_angle / float(neighboring_boids), -BOID_STEER_PER_TICK, BOID_STEER_PER_TICK);
        boid.direction = vec2(
            dot(boid.direction, vec2(cos(steer_angle), sin(steer_angle))),
            dot(boid.direction, vec2(-sin(steer_angle), cos(steer_angle)))
        );
    }

    update_boid_position(boid, old_boid);
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint invocation = gl_GlobalInvocationID.x;

    if (invocation >= MAX_BOIDS)
    {
        return;
    }

    BufferRef(Boids) boids_buffer = daxa_buffer_id_to_ref(Boids, BufferRef, push_constant.boids_buffer_id);
    BufferRef(Boids) old_boids_buffer = push_constant.old_boids_buffer;

    u64 address = daxa_buffer_ref_to_address(boids_buffer);
    boids_buffer = daxa_buffer_address_to_ref(Boids, BufferRef, address);

    update_boid(
        boids_buffer.boids[invocation],
        old_boids_buffer.boids[invocation],
        invocation,
        old_boids_buffer
    );
}