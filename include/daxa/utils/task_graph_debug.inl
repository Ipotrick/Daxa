#pragma once

#include <daxa/daxa.inl>

struct DebugTaskDrawDebugDisplayPush
{
    daxa_ImageViewIndex src;
    daxa_ImageViewIndex dst;
    daxa_u32vec2 src_size;
    daxa_u32 image_view_type;
    daxa_i32 format;
    daxa_f32 float_min;
    daxa_f32 float_max;
    daxa_i32 int_min;
    daxa_i32 int_max;
    daxa_u32 uint_min;
    daxa_u32 uint_max;
    daxa_i32 rainbow_ints;
    daxa_i32vec4 enabled_channels;
    daxa_i32vec2 mouse_over_index;
    daxa_BufferPtr(daxa_f32vec4) readback_ptr;
    daxa_u32 readback_index;
};
