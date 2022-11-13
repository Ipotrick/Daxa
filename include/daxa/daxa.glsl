#pragma once

#define daxa_b32 bool
#define daxa_i32 int
#define daxa_u32 uint
#define daxa_f32 float

#define daxa_b32vec2 bvec2
#define daxa_b32vec3 bvec3
#define daxa_b32vec4 bvec4
#define daxa_f32 float
#define daxa_f32vec2 vec2
#define daxa_f32mat2x2 mat2x2
#define daxa_f32mat2x3 mat2x3
#define daxa_f32mat2x4 mat2x4
#define daxa_f32vec3 vec3
#define daxa_f32mat3x2 mat3x2
#define daxa_f32mat3x3 mat3x3
#define daxa_f32mat3x4 mat3x4
#define daxa_f32vec4 vec4
#define daxa_f32mat4x2 mat4x2
#define daxa_f32mat4x3 mat4x3
#define daxa_f32mat4x4 mat4x4
#define daxa_i32 int
#define daxa_u32 uint
#define daxa_i64 int64_t
#define daxa_u64 uint64_t
#define daxa_i32vec2 ivec2
#define daxa_u32vec2 uvec2
#define daxa_i32vec3 ivec3
#define daxa_u32vec3 uvec3
#define daxa_i32vec4 ivec4
#define daxa_u32vec4 uvec4

struct daxa_BufferId
{
    daxa_u32 buffer_id_value;
};

struct daxa_ImageViewId
{
    daxa_u32 image_view_id_value;
};

struct daxa_ImageId
{
    daxa_u32 image_view_id_value;
};

struct daxa_SamplerId
{
    daxa_u32 sampler_id_value;
};

layout(scalar, binding = DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING, set = 0) readonly buffer daxa_BufferDeviceAddressBuffer
{
    daxa_u64 addresses[];
}
daxa_buffer_device_address_buffer;

#define DAXA_BUFFER_LAYOUT layout(scalar, binding = DAXA_STORAGE_BUFFER_BINDING, set = 0)
#define DAXA_BUFFER_REFERENCE_LAYOUT layout(scalar, buffer_reference, buffer_reference_align = 4)
    
#define DAXA_DECL_BUFFER(NAME, BODY) \
    DAXA_BUFFER_LAYOUT buffer daxa_BufferTableObject##NAME                                                             \
    BODY                                                                                                               \
    daxa_BufferTable##NAME[];                                                                                          \
    DAXA_BUFFER_LAYOUT readonly buffer daxa_ROBufferTableBlock##NAME                                                   \
    BODY                                                                                                               \
    daxa_ROBufferTable##NAME[];                                                                                        \
    DAXA_BUFFER_REFERENCE_LAYOUT buffer NAME##BufferRef BODY;                                                          \
    DAXA_BUFFER_REFERENCE_LAYOUT readonly buffer NAME##ROBufferRef BODY

#define DAXA_DECL_BUFFER_STRUCT(NAME, BODY)                                                                            \
    struct NAME BODY;                                                                                                  \
    DAXA_DECL_BUFFER(NAME, BODY)

#define DAXA_USE_PUSH_CONSTANT(NAME)                          \
    layout(scalar, push_constant) uniform _DAXA_PUSH_CONSTANT \
    {                                                         \
        NAME push_constant;                                   \
    };

#define daxa_BufferRef(STRUCT_TYPE) STRUCT_TYPE##BufferRef
#define daxa_ROBufferRef(STRUCT_TYPE) STRUCT_TYPE##ROBufferRef

#define daxa_buffer_ref_to_address(buffer_reference) u64(buffer_reference)
#define daxa_buffer_id_to_address(id) daxa_buffer_device_address_buffer.addresses[(DAXA_ID_INDEX_MASK & id.buffer_id_value)]
#define daxa_buffer_address_to_ref(STRUCT_TYPE, address) STRUCT_TYPE##BufferRef(address)
#define daxa_buffer_id_to_ref(STRUCT_TYPE, id) daxa_buffer_address_to_ref(STRUCT_TYPE, daxa_buffer_id_to_address(id))
#define daxa_buffer_address_to_roref(STRUCT_TYPE, address) STRUCT_TYPE##ROBufferRef(address)
#define daxa_buffer_id_to_roref(STRUCT_TYPE, id) daxa_buffer_address_to_roref(STRUCT_TYPE, daxa_buffer_id_to_address(id))

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

#define daxa_access_Buffer(STRUCT_TYPE, buffer_id) daxa_BufferTable##STRUCT_TYPE[(DAXA_ID_INDEX_MASK & buffer_id.buffer_id_value)]
#define daxa_access_ROBuffer(STRUCT_TYPE, buffer_id) daxa_ROBufferTable##STRUCT_TYPE[(DAXA_ID_INDEX_MASK & buffer_id.buffer_id_value)]
#define daxa_access_RWImage(IMAGE_TYPE, IMAGE_FORMAT, image_view_id) daxa_ReadWriteImageTable_##IMAGE_FORMAT##_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_access_CoherentRWImage(IMAGE_TYPE, FORMAT, image_view_id) daxa_CoherentReadWriteImageTable_##IMAGE_FORMAT##_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_access_Image(IMAGE_TYPE, image_view_id) daxa_ReadOnlyImageTable_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_access_CoherentImage(IMAGE_TYPE, image_view_id) daxa_CoherentReadOnlyImageTable_##IMAGE_TYPE[(DAXA_ID_INDEX_MASK & image_view_id.image_view_id_value)]
#define daxa_access_Sampler(SAMPLER_TYPE, sampler_id) daxa_SamplerTable##SAMPLER_TYPE[(DAXA_ID_INDEX_MASK & sampler_id.sampler_id_value)]

#ifdef DAXA_SHADER_NO_NAMESPACE_PRIMITIVES
#define b32 daxa_b32
#define i32 daxa_i32
#define u32 daxa_u32
#define f32 daxa_f32
#define b32vec2 daxa_b32vec2
#define b32vec3 daxa_b32vec3
#define b32vec4 daxa_b32vec4
#define f32 daxa_f32
#define f32vec2 daxa_f32vec2
#define f32mat2x2 daxa_f32mat2x2
#define f32mat2x3 daxa_f32mat2x3
#define f32mat2x4 daxa_f32mat2x4
#define f32vec3 daxa_f32vec3
#define f32mat3x2 daxa_f32mat3x2
#define f32mat3x3 daxa_f32mat3x3
#define f32mat3x4 daxa_f32mat3x4
#define f32vec4 daxa_f32vec4
#define f32mat4x2 daxa_f32mat4x2
#define f32mat4x3 daxa_f32mat4x3
#define f32mat4x4 daxa_f32mat4x4
#define i32 daxa_i32
#define u32 daxa_u32
#define i64 daxa_i64
#define u64 daxa_u64
#define i32vec2 daxa_i32vec2
#define u32vec2 daxa_u32vec2
#define i32vec3 daxa_i32vec3
#define u32vec3 daxa_u32vec3
#define i32vec4 daxa_i32vec4
#define u32vec4 daxa_u32vec4
#endif