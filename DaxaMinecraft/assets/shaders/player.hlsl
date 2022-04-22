#pragma once

struct PlayerInput {
    float2 mouse_delta;
    float delta_time;
    float fov;
    float mouse_sens;
    float speed, sprint_speed;
    uint move_flags;

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
    float3x3 view_mat;
};

struct Player {
    float3 pos, vel, rot;
    Camera camera;

    void update(in PlayerInput input) {
        camera.fov = input.fov;

        float delta_dist = input.speed * input.delta_time;

        rot.x -= input.mouse_delta.x * input.mouse_sens * 0.0001f * camera.fov;
        rot.y -= input.mouse_delta.y * input.mouse_sens * 0.0001f * camera.fov;

        const float MAX_ROT = 3.14159f / 2;
        if (rot.y > MAX_ROT)
            rot.y = MAX_ROT;
        if (rot.y < -MAX_ROT)
            rot.y = -MAX_ROT;

        float sin_rot_x = sin(rot.y), cos_rot_x = cos(rot.y);
        float sin_rot_y = sin(rot.x), cos_rot_y = cos(rot.x);
        float sin_rot_z = sin(rot.z), cos_rot_z = cos(rot.z);

        // clang-format off
        camera.view_mat = float3x3(
             1,          0,          0,
             0,  cos_rot_x,  sin_rot_x,
             0, -sin_rot_x,  cos_rot_x
        );
        float3x3 roty_mat = float3x3(
             cos_rot_y, 0, -sin_rot_y,
             0,         1,          0,
             sin_rot_y, 0,  cos_rot_y
        );
        // clang-format on

        camera.view_mat = mul(roty_mat, camera.view_mat);

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
