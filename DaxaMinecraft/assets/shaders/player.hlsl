#pragma once

struct PlayerInput {
    float2 mouse_delta;
    float delta_time;
    float fov;
    float mouse_sens;
    float speed, sprint_speed;
    u32 move_flags;

    bool is_moving() { return move_flags != 0; }
    bool move_sprint() { return move_flags & 0x01; }
    bool move_forward() { return move_flags & 0x02; }
    bool move_backward() { return move_flags & 0x04; }
    bool move_left() { return move_flags & 0x08; }
    bool move_right() { return move_flags & 0x10; }
    bool move_up() { return move_flags & 0x20; }
    bool move_down() { return move_flags & 0x40; }
};

struct Camera {
    float fov;
};

struct Player {
    float3 pos, vel, rot;
    Camera camera;

    void update(in PlayerInput input) {
        camera.fov = input.fov;

        float delta_dist = input.speed * input.delta_time;

        rot.x += input.mouse_delta.x * input.mouse_sens * 0.0001f * camera.fov;
        rot.y += input.mouse_delta.y * input.mouse_sens * 0.0001f * camera.fov;

        const float MAX_ROT = 3.14159f / 2;
        if (rot.y > MAX_ROT)
            rot.y = MAX_ROT)
        if (rot.y < -MAX_ROT)
            rot.y = -MAX_ROT)

        float sin_rot = sin(rot.x), cos_rot = cos(rot.x);

        if (input.move_sprint())
            delta_dist *= input.sprint_speed;
        if (input.move_forward())
            pos.z += sin_rot_x * delta_dist, pos.x -= cos_rot_x * delta_dist;
        if (input.move_backward())
            pos.z -= sin_rot_x * delta_dist, pos.x += cos_rot_x * delta_dist;
        if (input.move_left())
            pos.x += sin_rot_x * delta_dist, pos.z += cos_rot_x * delta_dist;
        if (input.move_right())
            pos.x -= sin_rot_x * delta_dist, pos.z -= cos_rot_x * delta_dist;
        if (input.move_up())
            pos.y -= delta_dist;
        if (input.move_down())
            pos.y += delta_dist;
    }
};
