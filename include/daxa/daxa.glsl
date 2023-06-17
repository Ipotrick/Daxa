#pragma once

// Daxa inl file type definitions:
#define daxa_b32 bool
#define daxa_b32vec1 daxa_b32
#define daxa_b32vec2 bvec2
#define daxa_b32vec3 bvec3
#define daxa_b32vec4 bvec4
#define daxa_f32 float
#define daxa_f32vec1 daxa_f32
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
#define daxa_i32vec1 daxa_i32
#define daxa_i32vec2 ivec2
#define daxa_i32vec3 ivec3
#define daxa_i32vec4 ivec4
#define daxa_u32 uint
#define daxa_u32vec1 daxa_u32
#define daxa_u32vec2 uvec2
#define daxa_u32vec3 uvec3
#define daxa_u32vec4 uvec4
#define daxa_i64 int64_t
#define daxa_i64vec1 daxa_i64
#define daxa_i64vec2 i64vec2
#define daxa_i64vec3 i64vec3
#define daxa_i64vec4 i64vec4
#define daxa_u64 uint64_t
#define daxa_u64vec1 daxa_u64
#define daxa_u64vec2 u64vec2
#define daxa_u64vec3 u64vec3
#define daxa_u64vec4 u64vec4

struct daxa_BufferId
{
    // Upper 8 bits contain the version.
    // Lower 24 bits contain the index.
    daxa_u32 value;
};
struct daxa_ImageViewId
{
    // Upper 8 bits contain the version.
    // Lower 24 bits contain the index.
    daxa_u32 value;
};
struct daxa_SamplerId
{
    // Upper 8 bits contain the version.
    // Lower 24 bits contain the index.
    daxa_u32 value;
};

/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_id_to_index(daxa_BufferId id)
{
    return (DAXA_ID_INDEX_MASK & id.value);
}

/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_id_to_index(daxa_ImageViewId id)
{
    return (DAXA_ID_INDEX_MASK & id.value);
}

/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_id_to_index(daxa_SamplerId id)
{
    return (DAXA_ID_INDEX_MASK & id.value);
}

// Daxa implementation detail begin
layout(scalar, binding = DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING, set = 0) readonly buffer daxa_BufferDeviceAddressBufferBlock { daxa_u64 addresses[]; }
daxa_buffer_device_address_buffer;
layout(binding = DAXA_SAMPLER_BINDING, set = 0) uniform sampler daxa_SamplerTable[];
layout(binding = DAXA_SAMPLER_BINDING, set = 0) uniform samplerShadow daxa_SamplerShadowTable[];
// Daxa implementation detail end

/// @brief Retrieves a buffer device address to the start of the buffer of the given buffer id.
/// @param buffer_id The buffer of which the buffer device address is retrieved for.
/// @return Buffer device address to the start of the buffer.
daxa_u64 daxa_id_to_address(daxa_BufferId buffer_id)
{
    return daxa_buffer_device_address_buffer.addresses[daxa_id_to_index(buffer_id)];
}

/// @brief  Pointer like syntax for a read write buffer device address blocks containing the given struct
///         The buffer reference block contains a single member called value of the given type.
///         These types are just redefines for bda blocks, so they have all the glsl syntax like casting to a u64 working.
/// @param STRUCT_TYPE Struct type contained by the buffer device address block / "pointed to type".
#define daxa_RWBufferPtr(STRUCT_TYPE) daxa_RWBufferPtr##STRUCT_TYPE
/// @brief  Pointer like syntax for a read only buffer device address blocks containing the given struct
///         The buffer reference block contains a single member called value of the given type.
///         These types are just redefines for bda blocks, so they have all the glsl syntax like casting to a u64 working.
/// @param STRUCT_TYPE Struct type contained by the buffer device address block / "pointed to type".
#define daxa_BufferPtr(STRUCT_TYPE) daxa_BufferPtr##STRUCT_TYPE
/// @brief  Pointer like syntax for a read write COHERENT buffer device address blocks containing the given struct
///         The buffer reference block contains a single member called value of the given type.
///         These types are just redefines for bda blocks, so they have all the glsl syntax like casting to a u64 working.
/// @param STRUCT_TYPE Struct type contained by the buffer device address block / "pointed to type".
#define daxa_CoherentRWBufferPtr(STRUCT_TYPE) daxa_CoherentRWBufferPtr$$STRUCT_TYPE
/// @brief  Defines a makro for more explicitly visible "dereferencing" of buffer pointers.
#define deref(BUFFER_PTR) BUFFER_PTR.value

/// @brief Defines the buffer reference layout used in all buffer references in daxa glsl.
#define DAXA_BUFFER_REFERENCE_LAYOUT layout(buffer_reference, scalar, buffer_reference_align = 4)
#define DAXA_DECL_BUFFER_REFERENCE(ALIGN) layout(buffer_reference, scalar, buffer_reference_align = ALIGN)
/// @brief Defines the storage image layout used in all buffer references in daxa glsl.
#define DAXA_STORAGE_IMAGE_LAYOUT layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0)
/// @brief Defines the sampled image layout used in all buffer references in daxa glsl.
#define DAXA_SAMPLED_IMAGE_LAYOUT layout(binding = DAXA_SAMPLED_IMAGE_BINDING, set = 0)
/// @brief Defines the sampler layout used in all buffer references in daxa glsl.
#define DAXA_SAMPLER_LAYOUT layout(binding = DAXA_SAMPLER_BINDING, set = 0)

/// @brief  Defines three buffer reference using daxas buffer reference layout.
///         The three blocks are 1. read write, 2. read only, 3. read write coherent.
///         The name of the buffer reference blocks are daxa_RWBufferPtr##STRUCT_TYPE daxa_BufferPtr##STRUCT_TYPE daxa_CoherentRWBufferPtr##STRUCT_TYPE.
///         The buffer reference block contains a single field called value with the given struct type.
/// @param STRUCT_TYPE Struct type of the value field in the buffer reference block.
/// Usage example:
///     struct T { daxa_u32 v; };
///     DAXA_DECL_BUFFER_PTR(T)
///     ...
///     void main()
///     {
///         daxa_BufferPtr(T) t_ptr0 = ...;
///         daxa_BufferPtrT   t_ptr1 = ...;
///     }
#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE)                                              \
    DAXA_BUFFER_REFERENCE_LAYOUT buffer daxa_RWBufferPtr##STRUCT_TYPE                  \
    {                                                                                  \
        STRUCT_TYPE value;                                                             \
    };                                                                                 \
    DAXA_BUFFER_REFERENCE_LAYOUT readonly buffer daxa_BufferPtr##STRUCT_TYPE           \
    {                                                                                  \
        STRUCT_TYPE value;                                                             \
    };                                                                                 \
    DAXA_BUFFER_REFERENCE_LAYOUT coherent buffer daxa_CoherentRWBufferPtr##STRUCT_TYPE \
    {                                                                                  \
        STRUCT_TYPE value;                                                             \
    };

#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN)    \
    DAXA_DECL_BUFFER_REFERENCE(ALIGN)                     \
    buffer daxa_RWBufferPtr##STRUCT_TYPE                  \
    {                                                     \
        STRUCT_TYPE value;                                \
    };                                                    \
    DAXA_DECL_BUFFER_REFERENCE(ALIGN)                     \
    readonly buffer daxa_BufferPtr##STRUCT_TYPE           \
    {                                                     \
        STRUCT_TYPE value;                                \
    };                                                    \
    DAXA_DECL_BUFFER_REFERENCE(ALIGN)                     \
    coherent buffer daxa_CoherentRWBufferPtr##STRUCT_TYPE \
    {                                                     \
        STRUCT_TYPE value;                                \
    };

/// @brief Defines a push constant using daxas predefined push constant layout.
/// @param STRUCT Struct type the push constant contains.
/// @param NAME Global name of the struct inside the push constant block.
/// Usage example:
///     struct T { daxa_u32 v; };
///     DAXA_DECL_PUSH_CONSTANT(T, push)
///     void main()
///     {
///         daxa_u32 v = push.v;
///     }
#define DAXA_DECL_PUSH_CONSTANT(STRUCT, NAME)                 \
    layout(push_constant, scalar) uniform _DAXA_PUSH_CONSTANT \
    {                                                         \
        STRUCT NAME;                                          \
    };

/// @brief  Can be used to define a constant buffer in inline or shader files.
///         Constant buffers are uniform buffers in glsl.
///         They can be useful in some cases for example when the gpu has hardware acceleration for uniform buffer bindings.
/// @param SLOT Represents the constant buffer binding slot used for the constant buffer.
/// Usage example:
///     DAXA_DECL_UNIFORM_BUFFER(0) ConstantBufferName { daxa_u32 value; };
///     ...
///     void main() {
///         daxa_u32 v = value; // value is globaly available in the shader.
///     }
///
#define DAXA_DECL_UNIFORM_BUFFER(SLOT) layout(set = DAXA_DECL_UNIFORM_BUFFER_BINDING_SET, binding = SLOT, buffer_reference_align = 4, scalar) uniform

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_GET_IMAGE(DIMENSION, image_id) daxa_image##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_IIMAGE(DIMENSION, image_id) daxa_iimage##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_UIMAGE(DIMENSION, image_id) daxa_uimage##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_I64IMAGE(DIMENSION, image_id) daxa_i64image##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_U64IMAGE(DIMENSION, image_id) daxa_u64image##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_COHERENT_IMAGE(DIMENSION, image_id) daxa_coherent_image##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_COHERENT_IIMAGE(DIMENSION, image_id) daxa_coherent_iimage##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_COHERENT_UIMAGE(DIMENSION, image_id) daxa_coherent_uimage##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_COHERENT_I64IMAGE(DIMENSION, image_id) daxa_coherent_i64image##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_COHERENT_U64IMAGE(DIMENSION, image_id) daxa_coherent_u64image##DIMENSION##Table[daxa_id_to_index(image_id)]

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_GET_TEXTURE(DIMENSION, image_id) daxa_texture##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_ITEXTURE(DIMENSION, image_id) daxa_itexture##DIMENSION##Table[daxa_id_to_index(image_id)]
#define _DAXA_GET_UTEXTURE(DIMENSION, image_id) daxa_utexture##DIMENSION##Table[daxa_id_to_index(image_id)]

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_GET_SAMPLER(DIMENSION, image_id, sampler_id) sampler##DIMENSION(_DAXA_GET_TEXTURE(DIMENSION, image_id), daxa_samplerTable[daxa_id_to_index(sampler_id)])
#define _DAXA_GET_ISAMPLER(DIMENSION, image_id, sampler_id) isampler##DIMENSION(_DAXA_GET_ITEXTURE(DIMENSION, image_id), daxa_samplerTable[daxa_id_to_index(sampler_id)])
#define _DAXA_GET_USAMPLER(DIMENSION, image_id, sampler_id) usampler##DIMENSION(_DAXA_GET_UTEXTURE(DIMENSION, image_id), daxa_samplerTable[daxa_id_to_index(sampler_id)])
#define _DAXA_GET_SAMPLERSHADOW(DIMENSION, image_id, sampler_id) sampler##DIMENSION(_DAXA_GET_TEXTURE(DIMENSION, image_id), daxa_samplerShadowTable[daxa_id_to_index(sampler_id)])
#define _DAXA_GET_ISAMPLERSHADOW(DIMENSION, image_id, sampler_id) sampler##DIMENSION##Shadow(_DAXA_GET_ITEXTURE(DIMENSION, image_id), daxa_samplerShadowTable[daxa_id_to_index(sampler_id)])
#define _DAXA_GET_USAMPLERSHADOW(DIMENSION, image_id, sampler_id) sampler##DIMENSION##Shadow(_DAXA_GET_UTEXTURE(DIMENSION, image_id), daxa_samplerShadowTable[daxa_id_to_index(sampler_id)])

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_DECL_IMAGE(DIMENSION)                                                                            \
    DAXA_STORAGE_IMAGE_LAYOUT uniform coherent image##DIMENSION daxa_coherent_image##DIMENSION##Table[];       \
    DAXA_STORAGE_IMAGE_LAYOUT uniform coherent iimage##DIMENSION daxa_coherent_iimage##DIMENSION##Table[];     \
    DAXA_STORAGE_IMAGE_LAYOUT uniform coherent uimage##DIMENSION daxa_coherent_uimage##DIMENSION##Table[];     \
    DAXA_STORAGE_IMAGE_LAYOUT uniform coherent i64image##DIMENSION daxa_coherent_i64image##DIMENSION##Table[]; \
    DAXA_STORAGE_IMAGE_LAYOUT uniform coherent u64image##DIMENSION daxa_coherent_u64image##DIMENSION##Table[]; \
    DAXA_STORAGE_IMAGE_LAYOUT uniform image##DIMENSION daxa_image##DIMENSION##Table[];                         \
    DAXA_STORAGE_IMAGE_LAYOUT uniform iimage##DIMENSION daxa_iimage##DIMENSION##Table[];                       \
    DAXA_STORAGE_IMAGE_LAYOUT uniform uimage##DIMENSION daxa_uimage##DIMENSION##Table[];                       \
    DAXA_STORAGE_IMAGE_LAYOUT uniform i64image##DIMENSION daxa_i64image##DIMENSION##Table[];                   \
    DAXA_STORAGE_IMAGE_LAYOUT uniform u64image##DIMENSION daxa_u64image##DIMENSION##Table[];                   \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform texture##DIMENSION daxa_texture##DIMENSION##Table[];                     \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform itexture##DIMENSION daxa_itexture##DIMENSION##Table[];                   \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform utexture##DIMENSION daxa_utexture##DIMENSION##Table[];

/// ONLY USED BY IMPLEMENTATION!
DAXA_SAMPLER_LAYOUT uniform sampler daxa_samplerTable[];
DAXA_SAMPLER_LAYOUT uniform samplerShadow daxa_samplerShadowTable[];

#define daxa_sampler(sampler_id) daxa_samplerTable[daxa_id_to_index(sampler_id)]
#define daxa_samplerShadow(sampler_id) daxa_samplerShadowTable[daxa_id_to_index(sampler_id)]

_DAXA_DECL_IMAGE(1D)
#define daxa_coherent_image1D(image_id) _DAXA_GET_COHERENT_IMAGE(1D, image_id)
#define daxa_coherent_iimage1D(image_id) _DAXA_GET_COHERENT_IIMAGE(1D, image_id)
#define daxa_coherent_uimage1D(image_id) _DAXA_GET_COHERENT_UIMAGE(1D, image_id)
#define daxa_coherent_i64image1D(image_id) _DAXA_GET_COHERENT_I64IMAGE(1D, image_id)
#define daxa_coherent_u64image1D(image_id) _DAXA_GET_COHERENT_U64IMAGE(1D, image_id)
#define daxa_image1D(image_id) _DAXA_GET_IMAGE(1D, image_id)
#define daxa_iimage1D(image_id) _DAXA_GET_IIMAGE(1D, image_id)
#define daxa_uimage1D(image_id) _DAXA_GET_UIMAGE(1D, image_id)
#define daxa_i64image1D(image_id) _DAXA_GET_I64IMAGE(1D, image_id)
#define daxa_u64image1D(image_id) _DAXA_GET_U64IMAGE(1D, image_id)
#define daxa_texture1D(image_id) _DAXA_GET_TEXTURE(1D, image_id)
#define daxa_itexture1D(image_id) _DAXA_GET_ITEXTURE(1D, image_id)
#define daxa_utexture1D(image_id) _DAXA_GET_UTEXTURE(1D, image_id)
#define daxa_sampler1D(image_id, sampler_id) _DAXA_GET_SAMPLER(1D, image_id, sampler_id)
#define daxa_isampler1D(image_id, sampler_id) _DAXA_GET_ISAMPLER(1D, image_id, sampler_id)
#define daxa_usampler1D(image_id, sampler_id) _DAXA_GET_USAMPLER(1D, image_id, sampler_id)
#define daxa_samplerShadow1D(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(1D, image_id, sampler_id)
#define daxa_isamplerShadow1D(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(1D, image_id, sampler_id)
#define daxa_usamplerShadow1D(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(1D, image_id, sampler_id)

_DAXA_DECL_IMAGE(2D)
#define daxa_coherent_image2D(image_id) _DAXA_GET_COHERENT_IMAGE(2D, image_id)
#define daxa_coherent_iimage2D(image_id) _DAXA_GET_COHERENT_IIMAGE(2D, image_id)
#define daxa_coherent_uimage2D(image_id) _DAXA_GET_COHERENT_UIMAGE(2D, image_id)
#define daxa_coherent_i64image2D(image_id) _DAXA_GET_COHERENT_I64IMAGE(2D, image_id)
#define daxa_coherent_u64image2D(image_id) _DAXA_GET_COHERENT_U64IMAGE(2D, image_id)
#define daxa_image2D(image_id) _DAXA_GET_IMAGE(2D, image_id)
#define daxa_iimage2D(image_id) _DAXA_GET_IIMAGE(2D, image_id)
#define daxa_uimage2D(image_id) _DAXA_GET_UIMAGE(2D, image_id)
#define daxa_i64image2D(image_id) _DAXA_GET_I64IMAGE(2D, image_id)
#define daxa_u64image2D(image_id) _DAXA_GET_U64IMAGE(2D, image_id)
#define daxa_texture2D(image_id) _DAXA_GET_TEXTURE(2D, image_id)
#define daxa_itexture2D(image_id) _DAXA_GET_ITEXTURE(2D, image_id)
#define daxa_utexture2D(image_id) _DAXA_GET_UTEXTURE(2D, image_id)
#define daxa_sampler2D(image_id, sampler_id) _DAXA_GET_SAMPLER(2D, image_id, sampler_id)
#define daxa_isampler2D(image_id, sampler_id) _DAXA_GET_ISAMPLER(2D, image_id, sampler_id)
#define daxa_usampler2D(image_id, sampler_id) _DAXA_GET_USAMPLER(2D, image_id, sampler_id)
#define daxa_sampler2DShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2D, image_id, sampler_id)
#define daxa_isampler2DShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2D, image_id, sampler_id)
#define daxa_usampler2DShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2D, image_id, sampler_id)

_DAXA_DECL_IMAGE(3D)
#define daxa_coherent_image3D(image_id) _DAXA_GET_COHERENT_IMAGE(3D, image_id)
#define daxa_coherent_iimage3D(image_id) _DAXA_GET_COHERENT_IIMAGE(3D, image_id)
#define daxa_coherent_uimage3D(image_id) _DAXA_GET_COHERENT_UIMAGE(3D, image_id)
#define daxa_coherent_i64image3D(image_id) _DAXA_GET_COHERENT_I64IMAGE(3D, image_id)
#define daxa_coherent_u64image3D(image_id) _DAXA_GET_COHERENT_U64IMAGE(3D, image_id)
#define daxa_image3D(image_id) _DAXA_GET_IMAGE(3D, image_id)
#define daxa_iimage3D(image_id) _DAXA_GET_IIMAGE(3D, image_id)
#define daxa_uimage3D(image_id) _DAXA_GET_UIMAGE(3D, image_id)
#define daxa_i64image3D(image_id) _DAXA_GET_I64IMAGE(3D, image_id)
#define daxa_u64image3D(image_id) _DAXA_GET_U64IMAGE(3D, image_id)
#define daxa_texture3D(image_id) _DAXA_GET_TEXTURE(3D, image_id)
#define daxa_itexture3D(image_id) _DAXA_GET_ITEXTURE(3D, image_id)
#define daxa_utexture3D(image_id) _DAXA_GET_UTEXTURE(3D, image_id)
#define daxa_sampler3D(image_id, sampler_id) _DAXA_GET_SAMPLER(3D, image_id, sampler_id)
#define daxa_isampler3D(image_id, sampler_id) _DAXA_GET_ISAMPLER(3D, image_id, sampler_id)
#define daxa_usampler3D(image_id, sampler_id) _DAXA_GET_USAMPLER(3D, image_id, sampler_id)
#define daxa_sampler3DShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(3D, image_id, sampler_id)
#define daxa_isampler3DShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(3D, image_id, sampler_id)
#define daxa_usampler3DShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(3D, image_id, sampler_id)

_DAXA_DECL_IMAGE(Cube)
#define daxa_coherent_imageCube(image_id) _DAXA_GET_COHERENT_IMAGE(Cube, image_id)
#define daxa_coherent_iimageCube(image_id) _DAXA_GET_COHERENT_IIMAGE(Cube, image_id)
#define daxa_coherent_uimageCube(image_id) _DAXA_GET_COHERENT_UIMAGE(Cube, image_id)
#define daxa_coherent_i64imageCube(image_id) _DAXA_GET_COHERENT_I64IMAGE(Cube, image_id)
#define daxa_coherent_u64imageCube(image_id) _DAXA_GET_COHERENT_U64IMAGE(Cube, image_id)
#define daxa_imageCube(image_id) _DAXA_GET_IMAGE(Cube, image_id)
#define daxa_iimageCube(image_id) _DAXA_GET_IIMAGE(Cube, image_id)
#define daxa_uimageCube(image_id) _DAXA_GET_UIMAGE(Cube, image_id)
#define daxa_i64imageCube(image_id) _DAXA_GET_I64IMAGE(Cube, image_id)
#define daxa_u64imageCube(image_id) _DAXA_GET_U64IMAGE(Cube, image_id)
#define daxa_textureCube(image_id) _DAXA_GET_TEXTURE(Cube, image_id)
#define daxa_itextureCube(image_id) _DAXA_GET_ITEXTURE(Cube, image_id)
#define daxa_utextureCube(image_id) _DAXA_GET_UTEXTURE(Cube, image_id)
#define daxa_samplerCube(image_id, sampler_id) _DAXA_GET_SAMPLER(Cube, image_id, sampler_id)
#define daxa_isamplerCube(image_id, sampler_id) _DAXA_GET_ISAMPLER(Cube, image_id, sampler_id)
#define daxa_usamplerCube(image_id, sampler_id) _DAXA_GET_USAMPLER(Cube, image_id, sampler_id)
#define daxa_samplerCubeShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(Cube, image_id, sampler_id)
#define daxa_isamplerCubeShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(Cube, image_id, sampler_id)
#define daxa_usamplerCubeShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(Cube, image_id, sampler_id)

_DAXA_DECL_IMAGE(CubeArray)
#define daxa_coherent_imageCubeArray(image_id) _DAXA_GET_COHERENT_IMAGE(CubeArray, image_id)
#define daxa_coherent_iimageCubeArray(image_id) _DAXA_GET_COHERENT_IIMAGE(CubeArray, image_id)
#define daxa_coherent_uimageCubeArray(image_id) _DAXA_GET_COHERENT_UIMAGE(CubeArray, image_id)
#define daxa_coherent_i64imageCubeArray(image_id) _DAXA_GET_COHERENT_I64IMAGE(CubeArray, image_id)
#define daxa_coherent_u64imageCubeArray(image_id) _DAXA_GET_COHERENT_U64IMAGE(CubeArray, image_id)
#define daxa_imageCubeArray(image_id) _DAXA_GET_IMAGE(CubeArray, image_id)
#define daxa_iimageCubeArray(image_id) _DAXA_GET_IIMAGE(CubeArray, image_id)
#define daxa_uimageCubeArray(image_id) _DAXA_GET_UIMAGE(CubeArray, image_id)
#define daxa_i64imageCubeArray(image_id) _DAXA_GET_I64IMAGE(CubeArray, image_id)
#define daxa_u64imageCubeArray(image_id) _DAXA_GET_U64IMAGE(CubeArray, image_id)
#define daxa_textureCubeArray(image_id) _DAXA_GET_TEXTURE(CubeArray, image_id)
#define daxa_itextureCubeArray(image_id) _DAXA_GET_ITEXTURE(CubeArray, image_id)
#define daxa_utextureCubeArray(image_id) _DAXA_GET_UTEXTURE(CubeArray, image_id)
#define daxa_samplerCubeArray(image_id, sampler_id) _DAXA_GET_SAMPLER(CubeArray, image_id, sampler_id)
#define daxa_isamplerCubeArray(image_id, sampler_id) _DAXA_GET_ISAMPLER(CubeArray, image_id, sampler_id)
#define daxa_usamplerCubeArray(image_id, sampler_id) _DAXA_GET_USAMPLER(CubeArray, image_id, sampler_id)
#define daxa_samplerCubeArrayShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(CubeArray, image_id, sampler_id)
#define daxa_isamplerCubeArrayShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(CubeArray, image_id, sampler_id)
#define daxa_usamplerCubeArrayShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(CubeArray, image_id, sampler_id)

_DAXA_DECL_IMAGE(1DArray)
#define daxa_coherent_image1DArray(image_id) _DAXA_GET_COHERENT_IMAGE(1DArray, image_id)
#define daxa_coherent_iimage1DArray(image_id) _DAXA_GET_COHERENT_IIMAGE(1DArray, image_id)
#define daxa_coherent_uimage1DArray(image_id) _DAXA_GET_COHERENT_UIMAGE(1DArray, image_id)
#define daxa_coherent_i64image1DArray(image_id) _DAXA_GET_COHERENT_I64IMAGE(1DArray, image_id)
#define daxa_coherent_u64image1DArray(image_id) _DAXA_GET_COHERENT_U64IMAGE(1DArray, image_id)
#define daxa_image1DArray(image_id) _DAXA_GET_IMAGE(1DArray, image_id)
#define daxa_iimage1DArray(image_id) _DAXA_GET_IIMAGE(1DArray, image_id)
#define daxa_uimage1DArray(image_id) _DAXA_GET_UIMAGE(1DArray, image_id)
#define daxa_i64image1DArray(image_id) _DAXA_GET_I64IMAGE(1DArray, image_id)
#define daxa_u64image1DArray(image_id) _DAXA_GET_U64IMAGE(1DArray, image_id)
#define daxa_texture1DArray(image_id) _DAXA_GET_TEXTURE(1DArray, image_id)
#define daxa_itexture1DArray(image_id) _DAXA_GET_ITEXTURE(1DArray, image_id)
#define daxa_utexture1DArray(image_id) _DAXA_GET_UTEXTURE(1DArray, image_id)
#define daxa_sampler1DArray(image_id, sampler_id) _DAXA_GET_SAMPLER(1DArray, image_id, sampler_id)
#define daxa_isampler1DArray(image_id, sampler_id) _DAXA_GET_ISAMPLER(1DArray, image_id, sampler_id)
#define daxa_usampler1DArray(image_id, sampler_id) _DAXA_GET_USAMPLER(1DArray, image_id, sampler_id)
#define daxa_sampler1DArrayShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(1DArray, image_id, sampler_id)
#define daxa_isampler1DArrayShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(1DArray, image_id, sampler_id)
#define daxa_usampler1DArrayShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(1DArray, image_id, sampler_id)

_DAXA_DECL_IMAGE(2DArray)
#define daxa_coherent_image2DArray(image_id) _DAXA_GET_COHERENT_IMAGE(2DArray, image_id)
#define daxa_coherent_iimage2DArray(image_id) _DAXA_GET_COHERENT_IIMAGE(2DArray, image_id)
#define daxa_coherent_uimage2DArray(image_id) _DAXA_GET_COHERENT_UIMAGE(2DArray, image_id)
#define daxa_coherent_i64image2DArray(image_id) _DAXA_GET_COHERENT_I64IMAGE(2DArray, image_id)
#define daxa_coherent_u64image2DArray(image_id) _DAXA_GET_COHERENT_U64IMAGE(2DArray, image_id)
#define daxa_image2DArray(image_id) _DAXA_GET_IMAGE(2DArray, image_id)
#define daxa_iimage2DArray(image_id) _DAXA_GET_IIMAGE(2DArray, image_id)
#define daxa_uimage2DArray(image_id) _DAXA_GET_UIMAGE(2DArray, image_id)
#define daxa_i64image2DArray(image_id) _DAXA_GET_I64IMAGE(2DArray, image_id)
#define daxa_u64image2DArray(image_id) _DAXA_GET_U64IMAGE(2DArray, image_id)
#define daxa_texture2DArray(image_id) _DAXA_GET_TEXTURE(2DArray, image_id)
#define daxa_itexture2DArray(image_id) _DAXA_GET_ITEXTURE(2DArray, image_id)
#define daxa_utexture2DArray(image_id) _DAXA_GET_UTEXTURE(2DArray, image_id)
#define daxa_sampler2DArray(image_id, sampler_id) _DAXA_GET_SAMPLER(2DArray, image_id, sampler_id)
#define daxa_isampler2DArray(image_id, sampler_id) _DAXA_GET_ISAMPLER(2DArray, image_id, sampler_id)
#define daxa_usampler2DArray(image_id, sampler_id) _DAXA_GET_USAMPLER(2DArray, image_id, sampler_id)
#define daxa_sampler2DArrayShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2DArray, image_id, sampler_id)
#define daxa_isampler2DArrayShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2DArray, image_id, sampler_id)
#define daxa_usampler2DArrayShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2DArray, image_id, sampler_id)

_DAXA_DECL_IMAGE(2DMS)
#define daxa_coherent_image2DMS(image_id) _DAXA_GET_COHERENT_IMAGE(2DMS, image_id)
#define daxa_coherent_iimage2DMS(image_id) _DAXA_GET_COHERENT_IIMAGE(2DMS, image_id)
#define daxa_coherent_uimage2DMS(image_id) _DAXA_GET_COHERENT_UIMAGE(2DMS, image_id)
#define daxa_coherent_i64image2DMS(image_id) _DAXA_GET_COHERENT_I64IMAGE(2DMS, image_id)
#define daxa_coherent_u64image2DMS(image_id) _DAXA_GET_COHERENT_U64IMAGE(2DMS, image_id)
#define daxa_image2DMS(image_id) _DAXA_GET_IMAGE(2DMS, image_id)
#define daxa_iimage2DMS(image_id) _DAXA_GET_IIMAGE(2DMS, image_id)
#define daxa_uimage2DMS(image_id) _DAXA_GET_UIMAGE(2DMS, image_id)
#define daxa_i64image2DMS(image_id) _DAXA_GET_I64IMAGE(2DMS, image_id)
#define daxa_u64image2DMS(image_id) _DAXA_GET_U64IMAGE(2DMS, image_id)
#define daxa_texture2DMS(image_id) _DAXA_GET_TEXTURE(2DMS, image_id)
#define daxa_itexture2DMS(image_id) _DAXA_GET_ITEXTURE(2DMS, image_id)
#define daxa_utexture2DMS(image_id) _DAXA_GET_UTEXTURE(2DMS, image_id)
#define daxa_sampler2DMS(image_id, sampler_id) _DAXA_GET_SAMPLER(2DMS, image_id, sampler_id)
#define daxa_isampler2DMS(image_id, sampler_id) _DAXA_GET_ISAMPLER(2DMS, image_id, sampler_id)
#define daxa_usampler2DMS(image_id, sampler_id) _DAXA_GET_USAMPLER(2DMS, image_id, sampler_id)
#define daxa_sampler2DMSShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2DMS, image_id, sampler_id)
#define daxa_isampler2DMSShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2DMS, image_id, sampler_id)
#define daxa_usampler2DMSShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2DMS, image_id, sampler_id)

_DAXA_DECL_IMAGE(2DMSArray)
#define daxa_coherent_image2DMSArray(image_id) _DAXA_GET_COHERENT_IMAGE(2DMSArray, image_id)
#define daxa_coherent_iimage2DMSArray(image_id) _DAXA_GET_COHERENT_IIMAGE(2DMSArray, image_id)
#define daxa_coherent_uimage2DMSArray(image_id) _DAXA_GET_COHERENT_UIMAGE(2DMSArray, image_id)
#define daxa_coherent_i64image2DMSArray(image_id) _DAXA_GET_COHERENT_I64IMAGE(2DMSArray, image_id)
#define daxa_coherent_u64image2DMSArray(image_id) _DAXA_GET_COHERENT_U64IMAGE(2DMSArray, image_id)
#define daxa_image2DMSArray(image_id) _DAXA_GET_IMAGE(2DMSArray, image_id)
#define daxa_iimage2DMSArray(image_id) _DAXA_GET_IIMAGE(2DMSArray, image_id)
#define daxa_uimage2DMSArray(image_id) _DAXA_GET_UIMAGE(2DMSArray, image_id)
#define daxa_i64image2DMSArray(image_id) _DAXA_GET_I64IMAGE(2DMSArray, image_id)
#define daxa_u64image2DMSArray(image_id) _DAXA_GET_U64IMAGE(2DMSArray, image_id)
#define daxa_texture2DMSArray(image_id) _DAXA_GET_TEXTURE(2DMSArray, image_id)
#define daxa_itexture2DMSArray(image_id) _DAXA_GET_ITEXTURE(2DMSArray, image_id)
#define daxa_utexture2DMSArray(image_id) _DAXA_GET_UTEXTURE(2DMSArray, image_id)
#define daxa_sampler2DMSArray(image_id, sampler_id) _DAXA_GET_SAMPLER(2DMSArray, image_id, sampler_id)
#define daxa_isampler2DMSArray(image_id, sampler_id) _DAXA_GET_ISAMPLER(2DMSArray, image_id, sampler_id)
#define daxa_usampler2DMSArray(image_id, sampler_id) _DAXA_GET_USAMPLER(2DMSArray, image_id, sampler_id)
#define daxa_sampler2DMSArrayShadow(image_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2DMSArray, image_id, sampler_id)
#define daxa_isampler2DMSArrayShadow(image_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2DMSArray, image_id, sampler_id)
#define daxa_usampler2DMSArrayShadow(image_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2DMSArray, image_id, sampler_id)

DAXA_DECL_BUFFER_PTR(daxa_b32)
DAXA_DECL_BUFFER_PTR(daxa_b32vec1)
DAXA_DECL_BUFFER_PTR(daxa_b32vec2)
DAXA_DECL_BUFFER_PTR(daxa_b32vec3)
DAXA_DECL_BUFFER_PTR(daxa_b32vec4)
DAXA_DECL_BUFFER_PTR(daxa_f32)
DAXA_DECL_BUFFER_PTR(daxa_f32vec1)
DAXA_DECL_BUFFER_PTR(daxa_f32vec2)
DAXA_DECL_BUFFER_PTR(daxa_f32mat2x2)
DAXA_DECL_BUFFER_PTR(daxa_f32mat2x3)
DAXA_DECL_BUFFER_PTR(daxa_f32mat2x4)
DAXA_DECL_BUFFER_PTR(daxa_f32vec3)
DAXA_DECL_BUFFER_PTR(daxa_f32mat3x2)
DAXA_DECL_BUFFER_PTR(daxa_f32mat3x3)
DAXA_DECL_BUFFER_PTR(daxa_f32mat3x4)
DAXA_DECL_BUFFER_PTR(daxa_f32vec4)
DAXA_DECL_BUFFER_PTR(daxa_f32mat4x2)
DAXA_DECL_BUFFER_PTR(daxa_f32mat4x3)
DAXA_DECL_BUFFER_PTR(daxa_f32mat4x4)
DAXA_DECL_BUFFER_PTR(daxa_i32)
DAXA_DECL_BUFFER_PTR(daxa_i32vec1)
DAXA_DECL_BUFFER_PTR(daxa_i32vec2)
DAXA_DECL_BUFFER_PTR(daxa_i32vec3)
DAXA_DECL_BUFFER_PTR(daxa_i32vec4)
DAXA_DECL_BUFFER_PTR(daxa_u32)
DAXA_DECL_BUFFER_PTR(daxa_u32vec1)
DAXA_DECL_BUFFER_PTR(daxa_u32vec2)
DAXA_DECL_BUFFER_PTR(daxa_u32vec3)
DAXA_DECL_BUFFER_PTR(daxa_u32vec4)
DAXA_DECL_BUFFER_PTR(daxa_i64)
DAXA_DECL_BUFFER_PTR(daxa_i64vec1)
DAXA_DECL_BUFFER_PTR(daxa_i64vec2)
DAXA_DECL_BUFFER_PTR(daxa_i64vec3)
DAXA_DECL_BUFFER_PTR(daxa_i64vec4)
DAXA_DECL_BUFFER_PTR(daxa_u64)
DAXA_DECL_BUFFER_PTR(daxa_u64vec1)
DAXA_DECL_BUFFER_PTR(daxa_u64vec2)
DAXA_DECL_BUFFER_PTR(daxa_u64vec3)

DAXA_DECL_BUFFER_PTR(daxa_BufferId)
DAXA_DECL_BUFFER_PTR(daxa_ImageViewId)
DAXA_DECL_BUFFER_PTR(daxa_SamplerId)

#if DAXA_ENABLE_SHADER_NO_NAMESPACE

#define BUFFER_REFERENCE_LAYOUT DAXA_BUFFER_REFERENCE_LAYOUT
#define STORAGE_IMAGE_LAYOUT DAXA_STORAGE_IMAGE_LAYOUT
#define SAMPLED_IMAGE_LAYOUT DAXA_SAMPLED_IMAGE_LAYOUT
#define SAMPLER_LAYOUT DAXA_SAMPLER_LAYOUT

#define BufferId daxa_BufferId
#define ImageViewId daxa_ImageViewId
#define SamplerId daxa_SamplerId

#define RWBufferPtr(STRUCT) daxa_RWBufferPtr##STRUCT
#define BufferPtr(STRUCT) daxa_BufferPtr##STRUCT
#define CoherentRWBufferPtr(STRUCT) daxa_CoherentRWBufferPtr##STRUCT

#define id_to_address daxa_id_to_address
#define id_to_index daxa_id_to_index

#define b32 daxa_b32
#define b32vec1 daxa_b32vec1
#define b32vec2 daxa_b32vec2
#define b32vec3 daxa_b32vec3
#define b32vec4 daxa_b32vec4
#define f32 daxa_f32
#define f32vec1 daxa_f32vec1
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
#define i32vec1 daxa_i32vec1
#define i32vec2 daxa_i32vec2
#define i32vec3 daxa_i32vec3
#define i32vec4 daxa_i32vec4
#define u32 daxa_u32
#define u32vec1 daxa_u32vec1
#define u32vec2 daxa_u32vec2
#define u32vec3 daxa_u32vec3
#define u32vec4 daxa_u32vec4
#define i64 daxa_i64
#define i64vec1 daxa_i64vec1
#define u64 daxa_u64
#define u64vec1 daxa_u64vec1
#endif