#pragma once

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#define b32 bool
#define i32 int
#define u32 uint
#define f32 float

#define b32vec2 bvec2
#define b32vec3 bvec3
#define b32vec4 bvec4
#define f32 float
#define f32vec2 vec2
#define f32mat2x2 mat2x2
#define f32mat2x3 mat2x3
#define f32mat2x4 mat2x4
#define f32vec3 vec3
#define f32mat3x2 mat3x2
#define f32mat3x3 mat3x3
#define f32mat3x4 mat3x4
#define f32vec4 vec4
#define f32mat4x2 mat4x2
#define f32mat4x3 mat4x3
#define f32mat4x4 mat4x4
#define i32 int
#define u32 uint
#define i32vec2 ivec2
#define u32vec2 uvec2
#define i32vec3 ivec3
#define u32vec3 uvec3
#define i32vec4 ivec4
#define u32vec4 uvec4

struct BufferId
{
    u32 buffer_id_value;
};

struct ImageViewId
{
    u32 image_view_id_value;
};

struct ImageId
{
    u32 image_view_id_value;
};

struct SamplerId
{
    u32 sampler_id_value;
};

#define DAXA_REGISTER_STRUCT_GET_BUFFER(STRUCT_TYPE)                                                                           \
    layout(scalar, binding = DAXA_STORAGE_BUFFER_BINDING, set = 0) buffer daxa_BufferTableObject##STRUCT_TYPE                  \
    {                                                                                                                          \
        STRUCT_TYPE value;                                                                                                     \
    }                                                                                                                          \
    daxa_BufferTable##STRUCT_TYPE[];                                                                                           \
    layout(scalar, binding = DAXA_STORAGE_BUFFER_BINDING, set = 0) coherent buffer daxa_CoherentBufferTableObject##STRUCT_TYPE \
    {                                                                                                                          \
        STRUCT_TYPE value;                                                                                                     \
    }                                                                                                                          \
    daxa_CoherentBufferTable##STRUCT_TYPE[];

#define _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, IMAGE_FORMAT)                                                                          \
    layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0, IMAGE_FORMAT) uniform IMAGE_TYPE daxa_ReadWriteImageTable_##IMAGE_FORMAT##_##IMAGE_TYPE[]; \
    layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0, IMAGE_FORMAT) coherent uniform IMAGE_TYPE daxa_CoherentReadWriteImageTable_##IMAGE_FORMAT##_##IMAGE_TYPE[];

#define DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(IMAGE_TYPE)             \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba32f)        \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba16f)        \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg32f)          \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg16f)          \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r11f_g11f_b10f) \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r32f)           \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r16f)           \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba16)         \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgb10_a2)       \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba8)          \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg16)           \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg8)            \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r16)            \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r8)             \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba16_snorm)   \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba8_snorm)    \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg16_snorm)     \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg8_snorm)      \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r16_snorm)      \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r8_snorm)

#define DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(IMAGE_TYPE)        \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba32i) \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba16i) \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba8i)  \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg32i)   \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg16i)   \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg8i)    \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r32i)    \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r16i)    \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r8i)

#define DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(IMAGE_TYPE)          \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba32ui)   \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba16ui)   \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgb10_a2ui) \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rgba8ui)    \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg32ui)     \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg16ui)     \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, rg8ui)      \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r32ui)      \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r16ui)      \
    _DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_IMPL(IMAGE_TYPE, r8ui)

#define DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(IMAGE_TYPE) \
    layout(binding = DAXA_SAMPLED_IMAGE_BINDING, set = 0) uniform IMAGE_TYPE daxa_ReadOnlyImageTable_##IMAGE_TYPE[];

#define DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE) \
    layout(binding = DAXA_SAMPLER_BINDING, set = 0) uniform SAMPLER_TYPE daxa_SamplerTable##SAMPLER_TYPE[];

#define DAXA_PUSH_CONSTANT(STRUCT_TYPE) \
    layout(push_constant) uniform _DAXA_PUSH_CONSTANT { STRUCT_TYPE daxa_push; };

DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(image1D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(image2D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(image3D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(imageCube)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(image1DArray)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(image2DArray)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_FLOAT(imageBuffer)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(iimage1D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(iimage2D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(iimage3D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(iimageCube)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(iimage1DArray)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(iimage2DArray)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_INT(iimageBuffer)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(uimage1D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(uimage2D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(uimage3D)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(uimageCube)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(uimage1DArray)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(uimage2DArray)
DAXA_REGISTER_READ_WRITE_IMAGE_TYPE_UINT(uimageBuffer)

DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(texture1D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(texture2D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(texture3D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(textureCube)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(texture1DArray)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(texture2DArray)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(textureBuffer)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(itexture1D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(itexture2D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(itexture3D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(itextureCube)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(itexture1DArray)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(itexture2DArray)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(itextureBuffer)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(utexture1D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(utexture2D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(utexture3D)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(utextureCube)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(utexture1DArray)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(utexture2DArray)
DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(utextureBuffer)

DAXA_REGISTER_SAMPLER_TYPE(sampler)
DAXA_REGISTER_SAMPLER_TYPE(sampler1D)
DAXA_REGISTER_SAMPLER_TYPE(sampler2D)
DAXA_REGISTER_SAMPLER_TYPE(sampler3D)
DAXA_REGISTER_SAMPLER_TYPE(samplerCube)
DAXA_REGISTER_SAMPLER_TYPE(sampler1DArray)
DAXA_REGISTER_SAMPLER_TYPE(sampler2DArray)
DAXA_REGISTER_SAMPLER_TYPE(samplerBuffer)
DAXA_REGISTER_SAMPLER_TYPE(isampler1D)
DAXA_REGISTER_SAMPLER_TYPE(isampler2D)
DAXA_REGISTER_SAMPLER_TYPE(isampler3D)
DAXA_REGISTER_SAMPLER_TYPE(isamplerCube)
DAXA_REGISTER_SAMPLER_TYPE(isampler1DArray)
DAXA_REGISTER_SAMPLER_TYPE(isampler2DArray)
DAXA_REGISTER_SAMPLER_TYPE(isamplerBuffer)
DAXA_REGISTER_SAMPLER_TYPE(usampler1D)
DAXA_REGISTER_SAMPLER_TYPE(usampler2D)
DAXA_REGISTER_SAMPLER_TYPE(usampler3D)
DAXA_REGISTER_SAMPLER_TYPE(usamplerCube)
DAXA_REGISTER_SAMPLER_TYPE(usampler1DArray)
DAXA_REGISTER_SAMPLER_TYPE(usampler2DArray)
DAXA_REGISTER_SAMPLER_TYPE(usamplerBuffer)
DAXA_REGISTER_SAMPLER_TYPE(sampler1DShadow)
DAXA_REGISTER_SAMPLER_TYPE(sampler2DShadow)
DAXA_REGISTER_SAMPLER_TYPE(samplerCubeShadow)
DAXA_REGISTER_SAMPLER_TYPE(sampler1DArrayShadow)
DAXA_REGISTER_SAMPLER_TYPE(sampler2DArrayShadow)

#define daxa_GetBuffer(STRUCT_TYPE, buffer_id) daxa_BufferTable##STRUCT_TYPE[(DAXA_ID_INDEX_MASK & buffer_id.buffer_id_value)].value
#define daxa_GetCoherentBuffer(STRUCT_TYPE, buffer_id) daxa_CoherentBufferTable##STRUCT_TYPE[(DAXA_ID_INDEX_MASK & buffer_id.buffer_id_value)].value
#define daxa_GetRWImage(IMAGE_TYPE, IMAGE_FORMAT, image_view_id) daxa_ReadWriteImageTable_##IMAGE_FORMAT##_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_GetCoherentRWImage(IMAGE_TYPE, FORMAT, image_view_id) daxa_CoherentReadWriteImageTable_##IMAGE_FORMAT##_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_GetImage(IMAGE_TYPE, image_view_id) daxa_ReadOnlyImageTable_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_GetCoherentImage(IMAGE_TYPE, image_view_id) daxa_CoherentReadOnlyImageTable_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_GetSampler(SAMPLER_TYPE, sampler_id) daxa_SamplerTable##SAMPLER_TYPE[(DAXA_ID_INDEX_MASK & sampler_id.sampler_id_value)]
