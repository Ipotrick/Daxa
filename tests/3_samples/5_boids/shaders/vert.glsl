#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <shared.inl>

DAXA_DECL_PUSH_CONSTANT(DrawPushConstant, push)

void main()
{
    uint boid_index = gl_VertexIndex / 3;

    if (boid_index >= MAX_BOIDS)
    {
        gl_Position = vec4(-1.0f, -1.0f, -1.0f, 1.0f);
        return;
    }

    vec2 corner_position;
    switch (gl_VertexIndex % 3)
    {
    case 0: corner_position = vec2(-1.0f, -1.0f); break;
    case 1: corner_position = vec2(1.0f, -1.0f); break;
    case 2: corner_position = vec2(0.0f, 1.0f); break;
    }

    vec2 direction = normalize(deref(push.boids_buffer).boids[boid_index].speed);
    direction = vec2(-direction.y, direction.x);
    direction = vec2(-direction.y, direction.x);
    direction = vec2(-direction.y, direction.x);

    corner_position = vec2(
        dot(corner_position, vec2(direction.x, -direction.y)),
        dot(corner_position, vec2(direction.y, direction.x)));

    corner_position *= BOID_SCALE;
    corner_position += deref(push.boids_buffer).boids[boid_index].position;
    corner_position *= 2.0f / (FIELD_SIZE);
    corner_position -= vec2(1.0f);
    corner_position *= push.axis_scaling;

    gl_Position = vec4(corner_position, 0.0f, 1.0f);
}