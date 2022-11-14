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

layout(scalar, binding = DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING, set = 0) readonly buffer daxa_BufferDeviceAddressBufferObject
{
    daxa_u64 addresses[];
}
daxa_buffer_device_address_buffer;

#define DAXA_BUFFER_LAYOUT layout(scalar, binding = DAXA_STORAGE_BUFFER_BINDING, set = 0)
#define DAXA_BUFFER_REFERENCE_LAYOUT layout(scalar, buffer_reference, buffer_reference_align = 4)
#define DAXA_STORAGE_IMAGE_LAYOUT layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0)
#define DAXA_SAMPLED_IMAGE_LAYOUT layout(binding = DAXA_SAMPLED_IMAGE_BINDING, set = 0)
#define DAXA_SAMPLER_LAYOUT layout(binding = DAXA_SAMPLER_BINDING, set = 0)
    
#define DAXA_DECL_BUFFER(NAME, BODY) \
    DAXA_BUFFER_LAYOUT buffer daxa_RWBufferTableBlock##NAME                                                            \
    BODY                                                                                                               \
    daxa_RWBufferTable##NAME[];                                                                                        \
    DAXA_BUFFER_LAYOUT readonly buffer daxa_ROBufferTableBlock##NAME                                                   \
    BODY                                                                                                               \
    daxa_ROBufferTable##NAME[];                                                                                        \
    DAXA_BUFFER_REFERENCE_LAYOUT buffer NAME##RWBufferRef BODY;                                                        \
    DAXA_BUFFER_REFERENCE_LAYOUT readonly buffer NAME##ROBufferRef BODY

#define DAXA_DECL_BUFFER_STRUCT(NAME, BODY)                                                                            \
    struct NAME BODY;                                                                                                  \
    DAXA_DECL_BUFFER(NAME, BODY)

#define DAXA_USE_PUSH_CONSTANT(NAME)                          \
    layout(scalar, push_constant) uniform _DAXA_PUSH_CONSTANT \
    {                                                         \
        NAME push_constant;                                   \
    };

#define daxa_BufferRef(STRUCT_TYPE) STRUCT_TYPE##RWBufferRef
#define daxa_ROBufferRef(STRUCT_TYPE) STRUCT_TYPE##ROBufferRef

#define daxa_buffer_ref_to_address(buffer_reference) u64(buffer_reference)
#define daxa_buffer_id_to_address(id) daxa_buffer_device_address_buffer.addresses[(DAXA_ID_INDEX_MASK & id.buffer_id_value)]
#define daxa_buffer_address_to_ref(STRUCT_TYPE, address) STRUCT_TYPE##RWBufferRef(address)
#define daxa_buffer_id_to_ref(STRUCT_TYPE, id) daxa_buffer_address_to_ref(STRUCT_TYPE, daxa_buffer_id_to_address(id))
#define daxa_buffer_address_to_roref(STRUCT_TYPE, address) STRUCT_TYPE##ROBufferRef(address)
#define daxa_buffer_id_to_roref(STRUCT_TYPE, id) daxa_buffer_address_to_roref(STRUCT_TYPE, daxa_buffer_id_to_address(id))

#define DAXA_REGISTER_IMAGE_TYPE(IMAGE_TYPE) \
    DAXA_STORAGE_IMAGE_LAYOUT readonly uniform IMAGE_TYPE daxa_ROImageTable##IMAGE_TYPE[]; \
    DAXA_STORAGE_IMAGE_LAYOUT uniform IMAGE_TYPE daxa_RWImageTable##IMAGE_TYPE[]; 

#define DAXA_REGISTER_TEXTURE_TYPE(TEXTURE_TYPE) \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform TEXTURE_TYPE daxa_TextureTable##TEXTURE_TYPE[];

#define DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE) \
    DAXA_SAMPLER_LAYOUT uniform SAMPLER_TYPE daxa_SamplerTable##SAMPLER_TYPE[];

DAXA_REGISTER_IMAGE_TYPE(image1D)
DAXA_REGISTER_IMAGE_TYPE(image2D)
DAXA_REGISTER_IMAGE_TYPE(image3D)
DAXA_REGISTER_IMAGE_TYPE(imageCube)
DAXA_REGISTER_IMAGE_TYPE(image1DArray)
DAXA_REGISTER_IMAGE_TYPE(image2DArray)
DAXA_REGISTER_IMAGE_TYPE(imageBuffer)
DAXA_REGISTER_IMAGE_TYPE(image1D)
DAXA_REGISTER_IMAGE_TYPE(image2D)
DAXA_REGISTER_IMAGE_TYPE(image3D)
DAXA_REGISTER_IMAGE_TYPE(imageCube)
DAXA_REGISTER_IMAGE_TYPE(image1DArray)
DAXA_REGISTER_IMAGE_TYPE(image2DArray)
DAXA_REGISTER_IMAGE_TYPE(imageBuffer)
DAXA_REGISTER_IMAGE_TYPE(iimage1D)
DAXA_REGISTER_IMAGE_TYPE(iimage2D)
DAXA_REGISTER_IMAGE_TYPE(iimage3D)
DAXA_REGISTER_IMAGE_TYPE(iimageCube)
DAXA_REGISTER_IMAGE_TYPE(iimage1DArray)
DAXA_REGISTER_IMAGE_TYPE(iimage2DArray)
DAXA_REGISTER_IMAGE_TYPE(iimageBuffer)
DAXA_REGISTER_IMAGE_TYPE(uimage1D)
DAXA_REGISTER_IMAGE_TYPE(uimage2D)
DAXA_REGISTER_IMAGE_TYPE(uimage3D)
DAXA_REGISTER_IMAGE_TYPE(uimageCube)
DAXA_REGISTER_IMAGE_TYPE(uimage1DArray)
DAXA_REGISTER_IMAGE_TYPE(uimage2DArray)
DAXA_REGISTER_IMAGE_TYPE(uimageBuffer)

DAXA_REGISTER_TEXTURE_TYPE(texture1D)
DAXA_REGISTER_TEXTURE_TYPE(texture2D)
DAXA_REGISTER_TEXTURE_TYPE(texture3D)
DAXA_REGISTER_TEXTURE_TYPE(textureCube)
DAXA_REGISTER_TEXTURE_TYPE(texture1DArray)
DAXA_REGISTER_TEXTURE_TYPE(texture2DArray)
DAXA_REGISTER_TEXTURE_TYPE(textureBuffer)
DAXA_REGISTER_TEXTURE_TYPE(itexture1D)
DAXA_REGISTER_TEXTURE_TYPE(itexture2D)
DAXA_REGISTER_TEXTURE_TYPE(itexture3D)
DAXA_REGISTER_TEXTURE_TYPE(itextureCube)
DAXA_REGISTER_TEXTURE_TYPE(itexture1DArray)
DAXA_REGISTER_TEXTURE_TYPE(itexture2DArray)
DAXA_REGISTER_TEXTURE_TYPE(itextureBuffer)
DAXA_REGISTER_TEXTURE_TYPE(utexture1D)
DAXA_REGISTER_TEXTURE_TYPE(utexture2D)
DAXA_REGISTER_TEXTURE_TYPE(utexture3D)
DAXA_REGISTER_TEXTURE_TYPE(utextureCube)
DAXA_REGISTER_TEXTURE_TYPE(utexture1DArray)
DAXA_REGISTER_TEXTURE_TYPE(utexture2DArray)
DAXA_REGISTER_TEXTURE_TYPE(utextureBuffer)

layout(binding = DAXA_SAMPLER_BINDING, set = 0) uniform sampler daxa_SamplerTable[];

#define daxa_id_to_index(ID) (DAXA_ID_INDEX_MASK & ID)

#define daxa_access_Buffer(STRUCT_TYPE, buffer_id) daxa_RWBufferTable##STRUCT_TYPE[]
#define daxa_access_ROBuffer(STRUCT_TYPE, buffer_id) daxa_ROBufferTable##STRUCT_TYPE[daxa_id_to_index(buffer_id.buffer_id_value)]
#define daxa_access_RWImage(IMAGE_TYPE, image_view_id) daxa_RWImageTable##IMAGE_TYPE[daxa_id_to_index(image_view_id.image_view_id_value)]
#define daxa_access_ROImage(IMAGE_TYPE, image_view_id) daxa_ROImageTable##IMAGE_TYPE[daxa_id_to_index(image_view_id.image_view_id_value)]
#define daxa_access_Texture(TEXTURE_TYPE, image_view_id) daxa_TextureTable##TEXTURE_TYPE[daxa_id_to_index(image_view_id.image_view_id_value)]
#define daxa_access_Sampler(sampler_id) daxa_SamplerTable[daxa_id_to_index(sampler_id.sampler_id_value)]

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