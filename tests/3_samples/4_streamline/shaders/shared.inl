#pragma once

#include "daxa/daxa.inl"

struct GpuInput
{
    daxa_u32vec2 frame_dim;
    daxa_f32vec3 camera_pos;
    daxa_f32vec3 camera_right;
    daxa_f32vec3 camera_up;
    daxa_f32vec3 camera_forward;
    float aspect;
    float fov;
    float near_plane;
    float far_plane;
};
DAXA_DECL_BUFFER_PTR(GpuInput)

struct ComputePush
{
    daxa::RWTexture2DId<daxa_f32vec4> image_id;
    daxa::RWTexture2DId<daxa_f32vec3> diffuse_albedo_id;
    daxa::RWTexture2DId<daxa_f32vec3> specular_albedo_id;
    daxa::RWTexture2DId<daxa_f32vec4> normal_roughness_id;
    daxa::RWTexture2DId<daxa_f32vec2> mvec_id;
    daxa::RWTexture2DId<daxa_f32vec2>  spec_mvec_id;
    daxa::RWTexture2DId<daxa_f32> depth_id;
    daxa::Texture2DId<daxa_f32vec4>  env_map;
    daxa::SamplerId env_sampler;
    daxa_BufferPtr(GpuInput) ptr;
    daxa_BufferPtr(GpuInput) prev_ptr;
    daxa_u32 frame_index;
    daxa_u32 render_mode;
};
