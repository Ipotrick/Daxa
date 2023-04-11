#pragma once

#include <daxa/daxa.inl>

#define LONG_LIFE_BUFFER_VALUE (33u)
#define LONG_LIFE_BUFFER_SIZE 8192u

#define MEDIUM_LIFE_IMAGE_VALUE (1.0f)
#define MEDIUM_LIFE_IMAGE_SIZE daxa_u32vec3(512u,200u,1u)

#define LONG_LIFE_IMAGE_VALUE (0.5f)
#define LONG_LIFE_IMAGE_SIZE daxa_u32vec3(32u,32u,32u)

#define SHORT_LIFE_IMAGE_VALUE (0.25f)
#define SHORT_LIFE_IMAGE_SIZE daxa_u32vec4(100u, 300u, 4u)

#define SHORT_LIFE_BUFFER_VALUE (12345678)
#define SHORT_LIFE_BUFFER_SIZE (64u)

struct TestImagePush
{
    daxa_Image3Df32 test_image;
    daxa_u32vec3 size;
    daxa_f32 value;
};

struct TestBufferPush
{
    daxa_BufferPtr(daxa_u32) test_buffer;
    daxa_u32 size;
    daxa_u32 value;
};