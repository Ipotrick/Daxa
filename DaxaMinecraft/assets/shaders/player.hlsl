#pragma once

#include "utils/intersect.hlsl"

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

    void update(in StructuredBuffer<Globals> globals, in PlayerInput input) {
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

        float4 motion_vec = float4(0, 0, 0, 0);

        if (input.move_sprint())
            delta_dist *= input.sprint_speed;
        if (input.move_forward())
            motion_vec += float4(-sin_rot_x * delta_dist, 0, cos_rot_x * delta_dist, 0);
        if (input.move_backward())
            motion_vec += float4(sin_rot_x * delta_dist, 0, -cos_rot_x * delta_dist, 0);
        if (input.move_left())
            motion_vec += float4(-cos_rot_x * delta_dist, 0, -sin_rot_x * delta_dist, 0);
        if (input.move_right())
            motion_vec += float4(cos_rot_x * delta_dist, 0, sin_rot_x * delta_dist, 0);
        if (input.move_up())
            motion_vec += float4(0, -delta_dist, 0, 0);
        if (input.move_down())
            motion_vec += float4(0, delta_dist, 0, 0);

        Ray ray;
        ray.o = globals[0].pos.xyz;
        ray.nrm = normalize(motion_vec.xyz);
        ray.inv_nrm = 1 / ray.nrm;
        RayIntersection view_chunk_intersection = trace_chunks(globals, ray);

        // float vec_length = length(motion_vec.xyz);
        // if (vec_length != 0) {
        //     motion_vec /= vec_length;
        //     if (view_chunk_intersection.hit) {
        //         vec_length = clamp(vec_length, 0, view_chunk_intersection.dist);
        //     }
        //     motion_vec *= vec_length;
        // }

        pos += motion_vec;
        pos.w = 0;
    }
};

struct PlayerBuffer {
    PlayerInput input;

    // ---- GPU ONLY ----

    Player player;
};

DAXA_DEFINE_BA_BUFFER(PlayerBuffer)
