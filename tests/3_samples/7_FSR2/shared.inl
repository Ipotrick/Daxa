#pragma once

#define DAXA_ENABLE_SHADER_NO_NAMESPACE 1
#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 1
#include <daxa/daxa.inl>

#include <voxels.inl>

struct PerframeInput
{
    daxa_f32mat4x4 view_mat;
    daxa_f32mat4x4 prev_view_mat;
    daxa_f32vec2 jitter;
};
DAXA_DECL_BUFFER_PTR(PerframeInput)

struct DrawPush
{
    daxa_BufferPtr(daxa_u32) packed_faces_ptr;
    daxa_BufferPtr(PerframeInput) perframe_input_ptr;
    daxa_Image2DArrayf32 atlas_texture;
    daxa_SamplerId atlas_sampler;
    daxa_f32vec3 chunk_pos;
};
