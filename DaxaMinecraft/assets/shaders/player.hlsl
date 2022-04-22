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
    bool move_left() { return move_flags & 0x02; }
    bool move_right() { return move_flags & 0x04; }
    bool move_forward() { return move_flags & 0x08; }
    bool move_backward() { return move_flags & 0x10; }
    bool move_up() { return move_flags & 0x20; }
    bool move_down() { return move_flags & 0x40; }
};

struct Camera {
    float4x4 view_mat;
    float fov;
};

struct Player {
    float4 pos;
    float4 vel;
    float4 rot;
    Camera camera;

    void update(in PlayerInput input) {
        float delta_dist = input.speed * input.delta_time;

        rot.x -= input.mouse_delta.x * input.mouse_sens * 0.0001f * input.fov;
        rot.y -= input.mouse_delta.y * input.mouse_sens * 0.0001f * input.fov;

        camera.fov = tan(input.fov * 3.14159f / 360.0f);

        const float MAX_ROT = 3.14159f / 2;
        if (rot.y > MAX_ROT)
            rot.y = MAX_ROT;
        if (rot.y < -MAX_ROT)
            rot.y = -MAX_ROT;

        float sin_rot_x = sin(rot.x), cos_rot_x = cos(rot.x);
        float sin_rot_y = sin(rot.y), cos_rot_y = cos(rot.y);
        float sin_rot_z = sin(rot.z), cos_rot_z = cos(rot.z);

        // clang-format off
        camera.view_mat = float4x4(
             1,          0,          0, 0,
             0,  cos_rot_y,  sin_rot_y, 0,
             0, -sin_rot_y,  cos_rot_y, 0,
             0,          0,          0, 0
        );
        float4x4 roty_mat = float4x4(
             cos_rot_x,  0, -sin_rot_x, 0,
             0,          1,          0, 0,
             sin_rot_x,  0,  cos_rot_x, 0,
             0,          0,          0, 0
        );
        // clang-format on

        camera.view_mat = mul(roty_mat, camera.view_mat);

        if (input.move_sprint())
            delta_dist *= input.sprint_speed;
        if (input.move_forward())
            pos.x -= sin_rot_x * delta_dist, pos.z += cos_rot_x * delta_dist;
        if (input.move_backward())
            pos.x += sin_rot_x * delta_dist, pos.z -= cos_rot_x * delta_dist;
        if (input.move_left())
            pos.z -= sin_rot_x * delta_dist, pos.x -= cos_rot_x * delta_dist;
        if (input.move_right())
            pos.z += sin_rot_x * delta_dist, pos.x += cos_rot_x * delta_dist;
        if (input.move_up())
            pos.y -= delta_dist;
        if (input.move_down())
            pos.y += delta_dist;
    }
};
