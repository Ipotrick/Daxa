#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#include <shared.inl>

DAXA_USE_PUSH_CONSTANT(UpdateBoidsPushConstant, push)

void update_boid(inout Boid boid, in const Boid old_boid, in uint boid_index, BufferPtr(Boids) old_boids_buffer)
{
    boid = old_boid;
    // Zero all accumulator variables
    // xpos_avg, ypos_avg, xvel_avg, yvel_avg, neighboring_boids, close_dx, close_dy = 0

    vec2 vis_other_pos_average = vec2(0,0);
    vec2 vis_other_vel_average = vec2(0,0);
    float vis_other_count = 0;
    vec2 vis_other_super_close_delta = vec2(0,0);
//
    // # For every other boid in the flock . . .
    // for each other boid (otherboid):
    for (uint other_i = 0; other_i < MAX_BOIDS; ++other_i)
    {
        if (other_i == boid_index)
        {
            continue;
        }
        Boid other = deref(old_boids_buffer).boids[other_i];
        // # Compute differences in x and y coordinates
        // dx = boid.x - otherboid.x
        // dy = boid.y - otherboid.y
        const vec2 pos_delta = boid.position - other.position;
        // # Are both those differences less than the visual range?
        // if (abs(dx)<visual_range and abs(dy)<visual_range):  
        // # Are both those differences less than the visual range?
        // if (abs(dx)<visual_range and abs(dy)<visual_range):  
        // # If so, calculate the squared distance
        // squared_distance = dx*dx + dy*dy
        const float squared_distance = dot(pos_delta,pos_delta);
        // # Is squared distance less than the protected range?
        // if (squared_distance < protected_range_squared):
        if (squared_distance < BOID_PROTECTED_RANGE_SQUARED)
        {
            // # If so, calculate difference in x/y-coordinates to nearfield boid
            // close_dx += boid.x - otherboid.x 
            // close_dy += boid.y - otherboid.y
            vis_other_super_close_delta += boid.position - other.position;
        }
        // # If not in protected range, is the boid in the visual range?
        // else if (squared_distance < visual_range_squared):
        else if (squared_distance < BOID_VIEW_RANGE_SQUARED)
        {
            // # Add other boid's x/y-coord and x/y vel to accumulator variables
            // xpos_avg += otherboid.x 
            // ypos_avg += otherboid.y 
            // xvel_avg += otherboid.vx
            // yvel_avg += otherboid.vy
            vis_other_pos_average += other.position;
            vis_other_vel_average += other.speed;
            // # Increment number of boids within visual range
            // neighboring_boids += 1 
            vis_other_count += 1;
        }
    }
    // # If there were any boids in the visual range . . .            
    // if (neighboring_boids > 0): 
    if (vis_other_count > 0.0f)
    {
        //# Divide accumulator variables by number of boids in visual range
        //xpos_avg = xpos_avg/neighboring_boids 
        //ypos_avg = ypos_avg/neighboring_boids
        //xvel_avg = xvel_avg/neighboring_boids
        //yvel_avg = yvel_avg/neighboring_boids
        vis_other_pos_average /= vis_other_count;
        vis_other_vel_average /= vis_other_count;
        //# Add the centering/matching contributions to velocity
        //boid.vx = (boid.vx + 
        //           (xpos_avg - boid.x)*centering_factor + 
        //           (xvel_avg - boid.vx)*matching_factor)
        //boid.vy = (boid.vy + 
        //           (ypos_avg - boid.y)*centering_factor + 
        //           (yvel_avg - boid.vy)*matching_factor)
        boid.speed = boid.speed + (vis_other_pos_average - boid.position) * BOID_COHERENCE + (vis_other_vel_average - boid.speed) * BOID_ALIGNMENT;
    }
    // # Add the avoidance contribution to velocity
    // boid.vx = boid.vx + (close_dx*avoidfactor)
    // boid.vy = boid.vy + (close_dy*avoidfactor)
    boid.speed += BOID_SEPERATION * vis_other_super_close_delta;
    //# If the boid is near an edge, make it turn by turnfactor
    //if outside top margin:
    //    boid.vy = boid.vy + turnfactor
    //if outside right margin:
    //    boid.vx = boid.vx - turnfactor
    //if outside left margin:
    //    boid.vx = boid.vx + turnfactor
    //if outside bottom margin:
    //    boid.vy = boid.vy - turnfactor
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
    // # Calculate the boid's speed
    // # Slow step! Lookup the "alpha max plus beta min" algorithm
    const float speed = max(0.01, length(boid.speed));
    //# Enforce min and max speeds
    //if speed < minspeed:
    //    boid.vx = (boid.vx/speed)*minspeed
    //    boid.vy = (boid.vy/speed)*maxspeed
    //if speed > maxspeed:
    //    boid.vx = (boid.vx/speed)*maxspeed
    //    boid.vy = (boid.vy/speed)*maxspeed
    if (speed < BOID_MIN_SPEED)
    {
        boid.speed = boid.speed / speed * BOID_MIN_SPEED;
    }
    if (speed > BOID_MAX_SPEED)
    {
        boid.speed = boid.speed / speed * BOID_MAX_SPEED;
    }

    // # Update boid's position
    // boid.x = boid.x + boid.vx
    // boid.y = boid.y + boid.vy
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