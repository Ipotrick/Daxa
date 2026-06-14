#ifndef DAXA_DAXA_GLSL
#define DAXA_DAXA_GLSL

// Optional features:
// #define DAXA_IMAGE_INT64
// #define DAXA_RAY_TRACING

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_KHR_shader_subgroup_basic : enable
#extension GL_KHR_shader_subgroup_vote : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_KHR_shader_subgroup_ballot : enable
#extension GL_KHR_shader_subgroup_shuffle : enable
#extension GL_KHR_shader_subgroup_shuffle_relative : enable
#extension GL_KHR_shader_subgroup_clustered : enable
#extension GL_KHR_shader_subgroup_quad : enable
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_image_load_formatted : require
#extension GL_EXT_control_flow_attributes : require
#extension GL_EXT_samplerless_texture_functions : require
#if DAXA_IMAGE_INT64
#extension GL_EXT_shader_image_int64 : require
#endif

#define DAXA_ID_INDEX_BITS 20
#define DAXA_ID_INDEX_MASK ((uint64_t(1) << DAXA_ID_INDEX_BITS) - uint64_t(1))
#define DAXA_ID_INDEX_OFFSTET 0
#define DAXA_ID_VERSION_BITS 44
#define DAXA_ID_VERSION_MASK ((uint64_t(1) << DAXA_ID_VERSION_BITS) - uint64_t(1))
#define DAXA_ID_VERSION_OFFSTET DAXA_ID_INDEX_BITS

#define DAXA_SHADER_STAGE_COMPUTE 0
#define DAXA_SHADER_STAGE_VERTEX 1
#define DAXA_SHADER_STAGE_TESSELATION_CONTROL 2
#define DAXA_SHADER_STAGE_TESSELATION_EVALUATION 3
#define DAXA_SHADER_STAGE_FRAGMENT 4
#define DAXA_SHADER_STAGE_TASK 5
#define DAXA_SHADER_STAGE_MESH 6
#define DAXA_SHADER_STAGE_RAYGEN 7
#define DAXA_SHADER_STAGE_ANY_HIT 8
#define DAXA_SHADER_STAGE_CLOSEST_HIT 9
#define DAXA_SHADER_STAGE_MISS 10
#define DAXA_SHADER_STAGE_INTERSECTION 11
#define DAXA_SHADER_STAGE_CALLABLE 12

// Daxa inl file type definitions:
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
#define daxa_b32 bool
#define daxa_b32vec1 daxa_b32
#define daxa_b32vec2 bvec2
#define daxa_b32vec3 bvec3
#define daxa_b32vec4 bvec4

struct daxa_BufferId
{
    uint64_t value;
};

struct daxa_ImageViewId
{
    uint64_t value;
};

struct daxa_ImageViewIndex
{
    daxa_u32 value;
};

struct daxa_SamplerId
{
    uint64_t value;
};

#if defined(DAXA_RAY_TRACING)
struct daxa_TlasId
{
    uint64_t value;
};
#endif

/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_buffer_id_to_index(daxa_BufferId id)
{
    return daxa_u32(id.value & DAXA_ID_INDEX_MASK);
}

/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_image_view_id_to_index(daxa_ImageViewId id)
{
    return daxa_u32(id.value & DAXA_ID_INDEX_MASK);
}

/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_image_view_id_to_index(daxa_ImageViewIndex index)
{
    return index.value;
}

/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_sampler_id_to_index(daxa_SamplerId id)
{
    return daxa_u32(id.value & DAXA_ID_INDEX_MASK);
}

#if defined(DAXA_RAY_TRACING)
/// @brief Every resource id contains an index and a version number. The index can be used to access the corresponding resource in the binding arrays/
/// @param id The id the index is retrieved from.
/// @return The index the id contains.
daxa_u32 daxa_acceleration_structure_id_to_index(daxa_TlasId id)
{
    return daxa_u32(id.value & DAXA_ID_INDEX_MASK);
}
#endif

// Daxa implementation detail begin
layout(scalar, binding = DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING, set = 0) restrict readonly buffer daxa_BufferDeviceAddressBufferBlock { daxa_u64 addresses[]; }
daxa_buffer_device_address_buffer;
// Daxa implementation detail end

/// @brief Retrieves a buffer device address to the start of the buffer of the given buffer id.
/// @param buffer_id The buffer of which the buffer device address is retrieved for.
/// @return Buffer device address to the start of the buffer.
daxa_u64 daxa_id_to_address(daxa_BufferId buffer_id)
{
    return daxa_buffer_device_address_buffer.addresses[daxa_buffer_id_to_index(buffer_id)];
}

#define _DAXA_BUFFER_PTR_INSTANTIATION_HELPER(DAXA_TYPE, STRUCT_TYPE) daxa_##DAXA_TYPE##STRUCT_TYPE
/// @brief  Pointer like syntax for a read write buffer device address blocks containing the given struct
///         The buffer reference block contains a single member called value of the given type.
///         These types are just redefines for bda blocks, so they have all the glsl syntax like casting to a u64 working.
/// @param STRUCT_TYPE Struct type contained by the buffer device address block / "pointed to type".
#define daxa_RWBufferPtr(STRUCT_TYPE) _DAXA_BUFFER_PTR_INSTANTIATION_HELPER(RWBufferPtr, STRUCT_TYPE)
/// @brief  Pointer like syntax for a read only buffer device address blocks containing the given struct
///         The buffer reference block contains a single member called value of the given type.
///         These types are just redefines for bda blocks, so they have all the glsl syntax like casting to a u64 working.
/// @param STRUCT_TYPE Struct type contained by the buffer device address block / "pointed to type".
#define daxa_BufferPtr(STRUCT_TYPE) _DAXA_BUFFER_PTR_INSTANTIATION_HELPER(BufferPtr, STRUCT_TYPE)
/// @brief  Defines a macro for more explicitly visible "dereferencing" of buffer pointers.
#define deref(BUFFER_PTR) (BUFFER_PTR).value
#define deref_i(BUFFER_PTR, INDEX) ((BUFFER_PTR + INDEX).value)
#define advance(BUFFER_PTR, offset) (BUFFER_PTR + offset)
#define as_address(x) uint64_t(x)

/// @brief Defines the buffer reference used in all buffer references in daxa glsl. Can also be used to declare new buffer references.
#define DAXA_DECL_BUFFER_REFERENCE_ALIGN(ALIGN) layout(buffer_reference, scalar, buffer_reference_align = ALIGN) buffer
#define DAXA_DECL_BUFFER_REFERENCE DAXA_DECL_BUFFER_REFERENCE_ALIGN(4)

/// @brief Defines the storage image layout used in all buffer references in daxa glsl with format specification.
#define DAXA_STORAGE_IMAGE_LAYOUT_WITH_FORMAT(FORMAT) layout(FORMAT, binding = DAXA_STORAGE_IMAGE_BINDING, set = 0)

/// @brief Defines the storage image layout used for all storage images in daxa glsl.
#define DAXA_STORAGE_IMAGE_LAYOUT layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0)
/// @brief Defines the sampled image layout used for all sampled images in daxa glsl.
#define DAXA_SAMPLED_IMAGE_LAYOUT layout(binding = DAXA_SAMPLED_IMAGE_BINDING, set = 0)
/// @brief Defines the sampler layout used for all samplers in daxa glsl.
#define DAXA_SAMPLER_LAYOUT layout(binding = DAXA_SAMPLER_BINDING, set = 0)
#if defined(DAXA_RAY_TRACING)
/// @brief Defines the acceleration structure layout used ifor all acceleration structures in daxa glsl.
#define DAXA_ACCELERATION_STRUCTURE_LAYOUT layout(binding = DAXA_ACCELERATION_STRUCTURE_BINDING, set = 0)
#endif

// Daxa implementation detail begin
DAXA_SAMPLER_LAYOUT uniform sampler daxa_SamplerTable[];
DAXA_SAMPLER_LAYOUT uniform samplerShadow daxa_SamplerShadowTable[];
#if defined(DAXA_RAY_TRACING)
DAXA_ACCELERATION_STRUCTURE_LAYOUT uniform accelerationStructureEXT daxa_AccelerationStructureTable[];
#endif
// Daxa implementation detail end

/// @brief  Defines three buffer reference using daxa's buffer reference layout.
///         The three blocks are 1. read write, 2. read only, 3. read write coherent.
///         The name of the buffer reference blocks are daxa_RWBufferPtr##STRUCT_TYPE daxa_BufferPtr##STRUCT_TYPE.
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
#define _DAXA_DECL_BUFFER_PTR_HELPER(STRUCT_TYPE)            \
    DAXA_DECL_BUFFER_REFERENCE daxa_RWBufferPtr##STRUCT_TYPE \
    {                                                        \
        STRUCT_TYPE value;                                   \
    };                                                       \
    DAXA_DECL_BUFFER_REFERENCE daxa_BufferPtr##STRUCT_TYPE   \
    {                                                        \
        readonly STRUCT_TYPE value;                          \
    };
#define _DAXA_FWD_DECL_BUFFER_PTR_HELPER(STRUCT_TYPE)         \
    DAXA_DECL_BUFFER_REFERENCE daxa_RWBufferPtr##STRUCT_TYPE; \
    DAXA_DECL_BUFFER_REFERENCE daxa_BufferPtr##STRUCT_TYPE;

#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE) _DAXA_DECL_BUFFER_PTR_HELPER(STRUCT_TYPE)

#define DAXA_FWD_DECL_BUFFER_PTR(STRUCT_TYPE) _DAXA_FWD_DECL_BUFFER_PTR_HELPER(STRUCT_TYPE)

#define _DAXA_DECL_BUFFER_PTR_ALIGN_HELPER(STRUCT_TYPE, ALIGN) \
    DAXA_DECL_BUFFER_REFERENCE_ALIGN(ALIGN)                    \
    daxa_RWBufferPtr##STRUCT_TYPE                              \
    {                                                          \
        STRUCT_TYPE value;                                     \
    };                                                         \
    DAXA_DECL_BUFFER_REFERENCE_ALIGN(ALIGN)                    \
    daxa_BufferPtr##STRUCT_TYPE                                \
    {                                                          \
        readonly STRUCT_TYPE value;                            \
    };
#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN) _DAXA_DECL_BUFFER_PTR_ALIGN_HELPER(STRUCT_TYPE, ALIGN)

/// @brief Defines a push constant using daxa's predefined push constant layout.
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

/// @brief  Can be used to define a specialized way to access image views.
///         Daxa only provides default accessors with no annotations, meaning that there is no way to get the glsl functionality of restrict, readonly, etc..
//          As the permutation count is gigantic, daxa does not predefine all possible accessors, instead the user can declare more specialized accessors themselfes.
///         DAXA_DECL_IMAGE_ACCESSOR declares a new accessor that can be used with daxa_access(ACCESSOR_NAME, image_view_id).
///         This can only be used for storage images.
/// @param TYPE image type, example image2D. Only storage image types allowed, for example texture2D would not be allowed.
/// @param ANNOTATIONS list of access specializations, example coherent.
/// @param ACCESSOR_NAME name of newly declared accessor, this name is used by daxa_access(ACCESSOR_NAME, image_view_id).
/// Usage example:
///     DAXA_DECL_IMAGE_ACCESSOR(image2D, coherent restrict, RWCoherRestr)
///     DAXA_DECL_IMAGE_ACCESSOR(iimage2DArray, writeonly restrict, WORestr)
///     ...
///     void main() {
///         vec4 v = imageLoad(daxa_access(RWCoherRestr, img0), ivec2(0,0));
///         imageStore(daxa_access(WORestr, img1), ivec2(0,0), 0, ivec4(v));
///     }
///
#define DAXA_DECL_IMAGE_ACCESSOR_WITH_FORMAT(TYPE, FORMAT, ANNOTATIONS, ACCESSOR_NAME) DAXA_STORAGE_IMAGE_LAYOUT_WITH_FORMAT(FORMAT) uniform ANNOTATIONS TYPE daxa_access_##ACCESSOR_NAME##Table[];
#define DAXA_DECL_IMAGE_ACCESSOR(TYPE, ANNOTATIONS, ACCESSOR_NAME) DAXA_STORAGE_IMAGE_LAYOUT uniform ANNOTATIONS TYPE daxa_access_##ACCESSOR_NAME##Table[];
#define daxa_access(ACCESSOR_NAME, image_view_id) daxa_access_##ACCESSOR_NAME##Table[daxa_image_view_id_to_index(image_view_id)]

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_GET_IMAGE(DIMENSION, image_view_id) daxa_image##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]
#define _DAXA_GET_IIMAGE(DIMENSION, image_view_id) daxa_iimage##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]
#define _DAXA_GET_UIMAGE(DIMENSION, image_view_id) daxa_uimage##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_GET_TEXTURE(DIMENSION, image_view_id) daxa_texture##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]
#define _DAXA_GET_ITEXTURE(DIMENSION, image_view_id) daxa_itexture##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]
#define _DAXA_GET_UTEXTURE(DIMENSION, image_view_id) daxa_utexture##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_GET_SAMPLER(DIMENSION, image_view_id, sampler_id) sampler##DIMENSION(_DAXA_GET_TEXTURE(DIMENSION, image_view_id), daxa_samplerTable[daxa_sampler_id_to_index(sampler_id)])
#define _DAXA_GET_ISAMPLER(DIMENSION, image_view_id, sampler_id) isampler##DIMENSION(_DAXA_GET_ITEXTURE(DIMENSION, image_view_id), daxa_samplerTable[daxa_sampler_id_to_index(sampler_id)])
#define _DAXA_GET_USAMPLER(DIMENSION, image_view_id, sampler_id) usampler##DIMENSION(_DAXA_GET_UTEXTURE(DIMENSION, image_view_id), daxa_samplerTable[daxa_sampler_id_to_index(sampler_id)])
#define _DAXA_GET_SAMPLERSHADOW(DIMENSION, image_view_id, sampler_id) sampler##DIMENSION##Shadow(_DAXA_GET_TEXTURE(DIMENSION, image_view_id), daxa_samplerShadowTable[daxa_sampler_id_to_index(sampler_id)])
#define _DAXA_GET_ISAMPLERSHADOW(DIMENSION, image_view_id, sampler_id) sampler##DIMENSION##Shadow(_DAXA_GET_ITEXTURE(DIMENSION, image_view_id), daxa_samplerShadowTable[daxa_sampler_id_to_index(sampler_id)])
#define _DAXA_GET_USAMPLERSHADOW(DIMENSION, image_view_id, sampler_id) sampler##DIMENSION##Shadow(_DAXA_GET_UTEXTURE(DIMENSION, image_view_id), daxa_samplerShadowTable[daxa_sampler_id_to_index(sampler_id)])

/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_DECL_IMAGE(DIMENSION)                                                          \
    DAXA_STORAGE_IMAGE_LAYOUT uniform image##DIMENSION daxa_image##DIMENSION##Table[];       \
    DAXA_STORAGE_IMAGE_LAYOUT uniform iimage##DIMENSION daxa_iimage##DIMENSION##Table[];     \
    DAXA_STORAGE_IMAGE_LAYOUT uniform uimage##DIMENSION daxa_uimage##DIMENSION##Table[];     \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform texture##DIMENSION daxa_texture##DIMENSION##Table[];   \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform itexture##DIMENSION daxa_itexture##DIMENSION##Table[]; \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform utexture##DIMENSION daxa_utexture##DIMENSION##Table[];

/// ONLY USED BY IMPLEMENTATION!
DAXA_SAMPLER_LAYOUT uniform sampler daxa_samplerTable[];
DAXA_SAMPLER_LAYOUT uniform samplerShadow daxa_samplerShadowTable[];

#define daxa_sampler(sampler_id) daxa_samplerTable[daxa_sampler_id_to_index(sampler_id)]
#define daxa_samplerShadow(sampler_id) daxa_samplerShadowTable[daxa_sampler_id_to_index(sampler_id)]

#if defined(DAXA_RAY_TRACING)
#define daxa_accelerationStructureEXT(as_id) daxa_AccelerationStructureTable[daxa_acceleration_structure_id_to_index(as_id)]
#endif

_DAXA_DECL_IMAGE(1D)
#define daxa_image1D(image_view_id) _DAXA_GET_IMAGE(1D, image_view_id)
#define daxa_iimage1D(image_view_id) _DAXA_GET_IIMAGE(1D, image_view_id)
#define daxa_uimage1D(image_view_id) _DAXA_GET_UIMAGE(1D, image_view_id)
#define daxa_texture1D(image_view_id) _DAXA_GET_TEXTURE(1D, image_view_id)
#define daxa_itexture1D(image_view_id) _DAXA_GET_ITEXTURE(1D, image_view_id)
#define daxa_utexture1D(image_view_id) _DAXA_GET_UTEXTURE(1D, image_view_id)
#define daxa_sampler1D(image_view_id, sampler_id) _DAXA_GET_SAMPLER(1D, image_view_id, sampler_id)
#define daxa_isampler1D(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(1D, image_view_id, sampler_id)
#define daxa_usampler1D(image_view_id, sampler_id) _DAXA_GET_USAMPLER(1D, image_view_id, sampler_id)
#define daxa_samplerShadow1D(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(1D, image_view_id, sampler_id)
#define daxa_isamplerShadow1D(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(1D, image_view_id, sampler_id)
#define daxa_usamplerShadow1D(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(1D, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(2D)
#define daxa_image2D(image_view_id) _DAXA_GET_IMAGE(2D, image_view_id)
#define daxa_iimage2D(image_view_id) _DAXA_GET_IIMAGE(2D, image_view_id)
#define daxa_uimage2D(image_view_id) _DAXA_GET_UIMAGE(2D, image_view_id)
#define daxa_texture2D(image_view_id) _DAXA_GET_TEXTURE(2D, image_view_id)
#define daxa_itexture2D(image_view_id) _DAXA_GET_ITEXTURE(2D, image_view_id)
#define daxa_utexture2D(image_view_id) _DAXA_GET_UTEXTURE(2D, image_view_id)
#define daxa_sampler2D(image_view_id, sampler_id) _DAXA_GET_SAMPLER(2D, image_view_id, sampler_id)
#define daxa_isampler2D(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(2D, image_view_id, sampler_id)
#define daxa_usampler2D(image_view_id, sampler_id) _DAXA_GET_USAMPLER(2D, image_view_id, sampler_id)
#define daxa_sampler2DShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2D, image_view_id, sampler_id)
#define daxa_isampler2DShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2D, image_view_id, sampler_id)
#define daxa_usampler2DShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2D, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(3D)
#define daxa_image3D(image_view_id) _DAXA_GET_IMAGE(3D, image_view_id)
#define daxa_iimage3D(image_view_id) _DAXA_GET_IIMAGE(3D, image_view_id)
#define daxa_uimage3D(image_view_id) _DAXA_GET_UIMAGE(3D, image_view_id)
#define daxa_texture3D(image_view_id) _DAXA_GET_TEXTURE(3D, image_view_id)
#define daxa_itexture3D(image_view_id) _DAXA_GET_ITEXTURE(3D, image_view_id)
#define daxa_utexture3D(image_view_id) _DAXA_GET_UTEXTURE(3D, image_view_id)
#define daxa_sampler3D(image_view_id, sampler_id) _DAXA_GET_SAMPLER(3D, image_view_id, sampler_id)
#define daxa_isampler3D(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(3D, image_view_id, sampler_id)
#define daxa_usampler3D(image_view_id, sampler_id) _DAXA_GET_USAMPLER(3D, image_view_id, sampler_id)
#define daxa_sampler3DShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(3D, image_view_id, sampler_id)
#define daxa_isampler3DShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(3D, image_view_id, sampler_id)
#define daxa_usampler3DShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(3D, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(Cube)
#define daxa_imageCube(image_view_id) _DAXA_GET_IMAGE(Cube, image_view_id)
#define daxa_iimageCube(image_view_id) _DAXA_GET_IIMAGE(Cube, image_view_id)
#define daxa_uimageCube(image_view_id) _DAXA_GET_UIMAGE(Cube, image_view_id)
#define daxa_textureCube(image_view_id) _DAXA_GET_TEXTURE(Cube, image_view_id)
#define daxa_itextureCube(image_view_id) _DAXA_GET_ITEXTURE(Cube, image_view_id)
#define daxa_utextureCube(image_view_id) _DAXA_GET_UTEXTURE(Cube, image_view_id)
#define daxa_samplerCube(image_view_id, sampler_id) _DAXA_GET_SAMPLER(Cube, image_view_id, sampler_id)
#define daxa_isamplerCube(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(Cube, image_view_id, sampler_id)
#define daxa_usamplerCube(image_view_id, sampler_id) _DAXA_GET_USAMPLER(Cube, image_view_id, sampler_id)
#define daxa_samplerCubeShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(Cube, image_view_id, sampler_id)
#define daxa_isamplerCubeShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(Cube, image_view_id, sampler_id)
#define daxa_usamplerCubeShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(Cube, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(CubeArray)
#define daxa_imageCubeArray(image_view_id) _DAXA_GET_IMAGE(CubeArray, image_view_id)
#define daxa_iimageCubeArray(image_view_id) _DAXA_GET_IIMAGE(CubeArray, image_view_id)
#define daxa_uimageCubeArray(image_view_id) _DAXA_GET_UIMAGE(CubeArray, image_view_id)
#define daxa_textureCubeArray(image_view_id) _DAXA_GET_TEXTURE(CubeArray, image_view_id)
#define daxa_itextureCubeArray(image_view_id) _DAXA_GET_ITEXTURE(CubeArray, image_view_id)
#define daxa_utextureCubeArray(image_view_id) _DAXA_GET_UTEXTURE(CubeArray, image_view_id)
#define daxa_samplerCubeArray(image_view_id, sampler_id) _DAXA_GET_SAMPLER(CubeArray, image_view_id, sampler_id)
#define daxa_isamplerCubeArray(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(CubeArray, image_view_id, sampler_id)
#define daxa_usamplerCubeArray(image_view_id, sampler_id) _DAXA_GET_USAMPLER(CubeArray, image_view_id, sampler_id)
#define daxa_samplerCubeArrayShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(CubeArray, image_view_id, sampler_id)
#define daxa_isamplerCubeArrayShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(CubeArray, image_view_id, sampler_id)
#define daxa_usamplerCubeArrayShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(CubeArray, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(1DArray)
#define daxa_image1DArray(image_view_id) _DAXA_GET_IMAGE(1DArray, image_view_id)
#define daxa_iimage1DArray(image_view_id) _DAXA_GET_IIMAGE(1DArray, image_view_id)
#define daxa_uimage1DArray(image_view_id) _DAXA_GET_UIMAGE(1DArray, image_view_id)
#define daxa_texture1DArray(image_view_id) _DAXA_GET_TEXTURE(1DArray, image_view_id)
#define daxa_itexture1DArray(image_view_id) _DAXA_GET_ITEXTURE(1DArray, image_view_id)
#define daxa_utexture1DArray(image_view_id) _DAXA_GET_UTEXTURE(1DArray, image_view_id)
#define daxa_sampler1DArray(image_view_id, sampler_id) _DAXA_GET_SAMPLER(1DArray, image_view_id, sampler_id)
#define daxa_isampler1DArray(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(1DArray, image_view_id, sampler_id)
#define daxa_usampler1DArray(image_view_id, sampler_id) _DAXA_GET_USAMPLER(1DArray, image_view_id, sampler_id)
#define daxa_sampler1DArrayShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(1DArray, image_view_id, sampler_id)
#define daxa_isampler1DArrayShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(1DArray, image_view_id, sampler_id)
#define daxa_usampler1DArrayShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(1DArray, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(2DArray)
#define daxa_image2DArray(image_view_id) _DAXA_GET_IMAGE(2DArray, image_view_id)
#define daxa_iimage2DArray(image_view_id) _DAXA_GET_IIMAGE(2DArray, image_view_id)
#define daxa_uimage2DArray(image_view_id) _DAXA_GET_UIMAGE(2DArray, image_view_id)
#define daxa_texture2DArray(image_view_id) _DAXA_GET_TEXTURE(2DArray, image_view_id)
#define daxa_itexture2DArray(image_view_id) _DAXA_GET_ITEXTURE(2DArray, image_view_id)
#define daxa_utexture2DArray(image_view_id) _DAXA_GET_UTEXTURE(2DArray, image_view_id)
#define daxa_sampler2DArray(image_view_id, sampler_id) _DAXA_GET_SAMPLER(2DArray, image_view_id, sampler_id)
#define daxa_isampler2DArray(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(2DArray, image_view_id, sampler_id)
#define daxa_usampler2DArray(image_view_id, sampler_id) _DAXA_GET_USAMPLER(2DArray, image_view_id, sampler_id)
#define daxa_sampler2DArrayShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2DArray, image_view_id, sampler_id)
#define daxa_isampler2DArrayShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2DArray, image_view_id, sampler_id)
#define daxa_usampler2DArrayShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2DArray, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(2DMS)
#define daxa_image2DMS(image_view_id) _DAXA_GET_IMAGE(2DMS, image_view_id)
#define daxa_iimage2DMS(image_view_id) _DAXA_GET_IIMAGE(2DMS, image_view_id)
#define daxa_uimage2DMS(image_view_id) _DAXA_GET_UIMAGE(2DMS, image_view_id)
#define daxa_texture2DMS(image_view_id) _DAXA_GET_TEXTURE(2DMS, image_view_id)
#define daxa_itexture2DMS(image_view_id) _DAXA_GET_ITEXTURE(2DMS, image_view_id)
#define daxa_utexture2DMS(image_view_id) _DAXA_GET_UTEXTURE(2DMS, image_view_id)
#define daxa_sampler2DMS(image_view_id, sampler_id) _DAXA_GET_SAMPLER(2DMS, image_view_id, sampler_id)
#define daxa_isampler2DMS(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(2DMS, image_view_id, sampler_id)
#define daxa_usampler2DMS(image_view_id, sampler_id) _DAXA_GET_USAMPLER(2DMS, image_view_id, sampler_id)
#define daxa_sampler2DMSShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2DMS, image_view_id, sampler_id)
#define daxa_isampler2DMSShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2DMS, image_view_id, sampler_id)
#define daxa_usampler2DMSShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2DMS, image_view_id, sampler_id)

_DAXA_DECL_IMAGE(2DMSArray)
#define daxa_image2DMSArray(image_view_id) _DAXA_GET_IMAGE(2DMSArray, image_view_id)
#define daxa_iimage2DMSArray(image_view_id) _DAXA_GET_IIMAGE(2DMSArray, image_view_id)
#define daxa_uimage2DMSArray(image_view_id) _DAXA_GET_UIMAGE(2DMSArray, image_view_id)
#define daxa_texture2DMSArray(image_view_id) _DAXA_GET_TEXTURE(2DMSArray, image_view_id)
#define daxa_itexture2DMSArray(image_view_id) _DAXA_GET_ITEXTURE(2DMSArray, image_view_id)
#define daxa_utexture2DMSArray(image_view_id) _DAXA_GET_UTEXTURE(2DMSArray, image_view_id)
#define daxa_sampler2DMSArray(image_view_id, sampler_id) _DAXA_GET_SAMPLER(2DMSArray, image_view_id, sampler_id)
#define daxa_isampler2DMSArray(image_view_id, sampler_id) _DAXA_GET_ISAMPLER(2DMSArray, image_view_id, sampler_id)
#define daxa_usampler2DMSArray(image_view_id, sampler_id) _DAXA_GET_USAMPLER(2DMSArray, image_view_id, sampler_id)
#define daxa_sampler2DMSArrayShadow(image_view_id, sampler_id) _DAXA_GET_SAMPLERSHADOW(2DMSArray, image_view_id, sampler_id)
#define daxa_isampler2DMSArrayShadow(image_view_id, sampler_id) _DAXA_GET_ISAMPLERSHADOW(2DMSArray, image_view_id, sampler_id)
#define daxa_usampler2DMSArrayShadow(image_view_id, sampler_id) _DAXA_GET_USAMPLERSHADOW(2DMSArray, image_view_id, sampler_id)

#if DAXA_IMAGE_INT64
/// ONLY USED BY IMPLEMENTATION!
#define _DAXA_DECL_IMAGE_INT64(DIMENSION)                                                    \
    DAXA_STORAGE_IMAGE_LAYOUT uniform i64image##DIMENSION daxa_i64image##DIMENSION##Table[]; \
    DAXA_STORAGE_IMAGE_LAYOUT uniform u64image##DIMENSION daxa_u64image##DIMENSION##Table[];

#define _DAXA_GET_I64IMAGE(DIMENSION, image_view_id) daxa_i64image##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]
#define _DAXA_GET_U64IMAGE(DIMENSION, image_view_id) daxa_u64image##DIMENSION##Table[daxa_image_view_id_to_index(image_view_id)]

_DAXA_DECL_IMAGE_INT64(1D)
#define daxa_i64image1D(image_view_id) _DAXA_GET_I64IMAGE(1D, image_view_id)
#define daxa_u64image1D(image_view_id) _DAXA_GET_U64IMAGE(1D, image_view_id)

_DAXA_DECL_IMAGE_INT64(2D)
#define daxa_i64image2D(image_view_id) _DAXA_GET_I64IMAGE(2D, image_view_id)
#define daxa_u64image2D(image_view_id) _DAXA_GET_U64IMAGE(2D, image_view_id)

_DAXA_DECL_IMAGE_INT64(3D)
#define daxa_i64image3D(image_view_id) _DAXA_GET_I64IMAGE(3D, image_view_id)
#define daxa_u64image3D(image_view_id) _DAXA_GET_U64IMAGE(3D, image_view_id)

_DAXA_DECL_IMAGE_INT64(1DArray)
#define daxa_i64image1DArray(image_view_id) _DAXA_GET_I64IMAGE(1DArray, image_view_id)
#define daxa_u64image1DArray(image_view_id) _DAXA_GET_U64IMAGE(1DArray, image_view_id)

_DAXA_DECL_IMAGE_INT64(2DArray)
#define daxa_i64image2DArray(image_view_id) _DAXA_GET_I64IMAGE(2DArray, image_view_id)
#define daxa_u64image2DArray(image_view_id) _DAXA_GET_U64IMAGE(2DArray, image_view_id)
#endif

DAXA_DECL_BUFFER_PTR(daxa_f32)
// DAXA_DECL_BUFFER_PTR(daxa_f32vec1) // covered by daxa_f32
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
// DAXA_DECL_BUFFER_PTR(daxa_i32vec1) // covered by daxa_i32
DAXA_DECL_BUFFER_PTR(daxa_i32vec2)
DAXA_DECL_BUFFER_PTR(daxa_i32vec3)
DAXA_DECL_BUFFER_PTR(daxa_i32vec4)
DAXA_DECL_BUFFER_PTR(daxa_u32)
// DAXA_DECL_BUFFER_PTR(daxa_u32vec1) // covered by daxa_u32
DAXA_DECL_BUFFER_PTR(daxa_u32vec2)
DAXA_DECL_BUFFER_PTR(daxa_u32vec3)
DAXA_DECL_BUFFER_PTR(daxa_u32vec4)
DAXA_DECL_BUFFER_PTR(daxa_i64)
// DAXA_DECL_BUFFER_PTR(daxa_i64vec1) // covered by daxa_i64
DAXA_DECL_BUFFER_PTR(daxa_i64vec2)
DAXA_DECL_BUFFER_PTR(daxa_i64vec3)
DAXA_DECL_BUFFER_PTR(daxa_i64vec4)
DAXA_DECL_BUFFER_PTR(daxa_u64)
// DAXA_DECL_BUFFER_PTR(daxa_u64vec1) // covered by daxa_u64
DAXA_DECL_BUFFER_PTR(daxa_u64vec2)
DAXA_DECL_BUFFER_PTR(daxa_u64vec3)

DAXA_DECL_BUFFER_PTR(daxa_BufferId)
DAXA_DECL_BUFFER_PTR(daxa_ImageViewId)
DAXA_DECL_BUFFER_PTR(daxa_SamplerId)

#endif
