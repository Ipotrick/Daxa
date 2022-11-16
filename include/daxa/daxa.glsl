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

#define daxa_BufferRef(BUFFER_STRUCT_TYPE) BUFFER_STRUCT_TYPE##RWBufferRef
#define daxa_ROBufferRef(BUFFER_STRUCT_TYPE) BUFFER_STRUCT_TYPE##ROBufferRef

#define daxa_buffer_ref_to_address(buffer_reference) u64(buffer_reference)
#define daxa_buffer_id_to_address(id) daxa_buffer_device_address_buffer.addresses[(DAXA_ID_INDEX_MASK & id.buffer_id_value)]
#define daxa_buffer_address_to_ref(BUFFER_STRUCT_TYPE, address) BUFFER_STRUCT_TYPE##RWBufferRef(address)
#define daxa_buffer_id_to_ref(BUFFER_STRUCT_TYPE, id) daxa_buffer_address_to_ref(BUFFER_STRUCT_TYPE, daxa_buffer_id_to_address(id))
#define daxa_buffer_address_to_roref(BUFFER_STRUCT_TYPE, address) BUFFER_STRUCT_TYPE##ROBufferRef(address)
#define daxa_buffer_id_to_roref(BUFFER_STRUCT_TYPE, id) daxa_buffer_address_to_roref(BUFFER_STRUCT_TYPE, daxa_buffer_id_to_address(id))

#define _DAXA_REGISTER_IMAGE_TYPE(IMAGE_TYPE) \
    DAXA_STORAGE_IMAGE_LAYOUT readonly uniform IMAGE_TYPE daxa_ROImageTable##IMAGE_TYPE[]; \
    DAXA_STORAGE_IMAGE_LAYOUT uniform IMAGE_TYPE daxa_RWImageTable##IMAGE_TYPE[]; 

#define _DAXA_REGISTER_TEXTURE_TYPE(TEXTURE_TYPE) \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform TEXTURE_TYPE daxa_TextureTable##TEXTURE_TYPE[];

_DAXA_REGISTER_IMAGE_TYPE(image1D)
_DAXA_REGISTER_IMAGE_TYPE(image2D)
_DAXA_REGISTER_IMAGE_TYPE(image3D)
_DAXA_REGISTER_IMAGE_TYPE(imageCube)
_DAXA_REGISTER_IMAGE_TYPE(image1DArray)
_DAXA_REGISTER_IMAGE_TYPE(image2DArray)
_DAXA_REGISTER_IMAGE_TYPE(imageBuffer)
_DAXA_REGISTER_IMAGE_TYPE(image1D)
_DAXA_REGISTER_IMAGE_TYPE(image2D)
_DAXA_REGISTER_IMAGE_TYPE(image3D)
_DAXA_REGISTER_IMAGE_TYPE(imageCube)
_DAXA_REGISTER_IMAGE_TYPE(image1DArray)
_DAXA_REGISTER_IMAGE_TYPE(image2DArray)
_DAXA_REGISTER_IMAGE_TYPE(imageBuffer)
_DAXA_REGISTER_IMAGE_TYPE(iimage1D)
_DAXA_REGISTER_IMAGE_TYPE(iimage2D)
_DAXA_REGISTER_IMAGE_TYPE(iimage3D)
_DAXA_REGISTER_IMAGE_TYPE(iimageCube)
_DAXA_REGISTER_IMAGE_TYPE(iimage1DArray)
_DAXA_REGISTER_IMAGE_TYPE(iimage2DArray)
_DAXA_REGISTER_IMAGE_TYPE(iimageBuffer)
_DAXA_REGISTER_IMAGE_TYPE(uimage1D)
_DAXA_REGISTER_IMAGE_TYPE(uimage2D)
_DAXA_REGISTER_IMAGE_TYPE(uimage3D)
_DAXA_REGISTER_IMAGE_TYPE(uimageCube)
_DAXA_REGISTER_IMAGE_TYPE(uimage1DArray)
_DAXA_REGISTER_IMAGE_TYPE(uimage2DArray)
_DAXA_REGISTER_IMAGE_TYPE(uimageBuffer)

_DAXA_REGISTER_TEXTURE_TYPE(texture1D)
_DAXA_REGISTER_TEXTURE_TYPE(texture2D)
_DAXA_REGISTER_TEXTURE_TYPE(texture3D)
_DAXA_REGISTER_TEXTURE_TYPE(textureCube)
_DAXA_REGISTER_TEXTURE_TYPE(texture1DArray)
_DAXA_REGISTER_TEXTURE_TYPE(texture2DArray)
_DAXA_REGISTER_TEXTURE_TYPE(textureBuffer)
_DAXA_REGISTER_TEXTURE_TYPE(itexture1D)
_DAXA_REGISTER_TEXTURE_TYPE(itexture2D)
_DAXA_REGISTER_TEXTURE_TYPE(itexture3D)
_DAXA_REGISTER_TEXTURE_TYPE(itextureCube)
_DAXA_REGISTER_TEXTURE_TYPE(itexture1DArray)
_DAXA_REGISTER_TEXTURE_TYPE(itexture2DArray)
_DAXA_REGISTER_TEXTURE_TYPE(itextureBuffer)
_DAXA_REGISTER_TEXTURE_TYPE(utexture1D)
_DAXA_REGISTER_TEXTURE_TYPE(utexture2D)
_DAXA_REGISTER_TEXTURE_TYPE(utexture3D)
_DAXA_REGISTER_TEXTURE_TYPE(utextureCube)
_DAXA_REGISTER_TEXTURE_TYPE(utexture1DArray)
_DAXA_REGISTER_TEXTURE_TYPE(utexture2DArray)
_DAXA_REGISTER_TEXTURE_TYPE(utextureBuffer)

layout(binding = DAXA_SAMPLER_BINDING, set = 0) uniform sampler daxa_SamplerTable[];

#define daxa_id_to_index(ID) (DAXA_ID_INDEX_MASK & ID)

#define daxa_get_buffer(BUFFER_STRUCT_TYPE, buffer_id) daxa_RWBufferTable##BUFFER_STRUCT_TYPE[]
#define daxa_get_readonly_buffer(BUFFER_STRUCT_TYPE, buffer_id) daxa_ROBufferTable##BUFFER_STRUCT_TYPE[daxa_id_to_index(buffer_id.buffer_id_value)]
#define daxa_get_image(IMAGE_TYPE, image_view_id) daxa_RWImageTable##IMAGE_TYPE[daxa_id_to_index(image_view_id.image_view_id_value)]
#define daxa_get_readonly_image(IMAGE_TYPE, image_view_id) daxa_ROImageTable##IMAGE_TYPE[daxa_id_to_index(image_view_id.image_view_id_value)]
#define daxa_get_texture(TEXTURE_TYPE, image_view_id) daxa_TextureTable##TEXTURE_TYPE[daxa_id_to_index(image_view_id.image_view_id_value)]
#define daxa_get_sampler(sampler_id) daxa_SamplerTable[daxa_id_to_index(sampler_id.sampler_id_value)]


#ifdef DAXA_SHADER_NO_NAMESPACE 
#define DAXA_SHADER_NO_NAMESPACE_PRIMITIVES

#define BufferId daxa_BufferId
#define ImageViewId daxa_ImageViewId
#define ImageId daxa_ImageId
#define SamplerId daxa_SamplerId

#define BUFFER_LAYOUT DAXA_BUFFER_LAYOUT
#define BUFFER_REFERENCE_LAYOUT DAXA_BUFFER_REFERENCE_LAYOUT
#define STORAGE_IMAGE_LAYOUT DAXA_STORAGE_IMAGE_LAYOUT
#define SAMPLED_IMAGE_LAYOUT DAXA_SAMPLED_IMAGE_LAYOUT
#define SAMPLER_LAYOUT DAXA_SAMPLER_LAYOUT

#define BufferRef daxa_BufferRef
#define ROBufferRef daxa_ROBufferRef

#define buffer_ref_to_address daxa_buffer_ref_to_address
#define buffer_id_to_address daxa_buffer_id_to_address
#define buffer_address_to_ref daxa_buffer_address_to_ref
#define buffer_id_to_ref daxa_buffer_id_to_ref
#define buffer_address_to_roref daxa_buffer_address_to_roref
#define buffer_id_to_roref daxa_buffer_id_to_roref

#define get_buffer daxa_get_buffer
#define get_readonly_buffer daxa_get_readonly_buffer
#define get_image daxa_get_image
#define get_readonly_image daxa_get_readonly_image
#define get_texture daxa_get_texture
#define get_sampler daxa_get_sampler

#endif


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