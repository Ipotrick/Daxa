#pragma once

#if !defined(DAXA_STORAGE_BUFFER_BINDING)
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ID_INDEX_MASK (0x00FFFFFF)
#endif

//
// Optional defines, activating certain features:
// * DAXA_ENABLE_IMAGE_OVERLOADS_BASIC
// * DAXA_ENABLE_IMAGE_OVERLOADS_ATOMIC
// * DAXA_ENABLE_IMAGE_OVERLOADS_64BIT
// * DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE
// * DAXA_ENABLE_SHADER_NO_NAMESPACE
// * DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES
//

#define daxa_b32 bool
#define daxa_f32 float
#define daxa_i32 int
#define daxa_u32 uint

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

// The ids on the host side can be brought to the gpu and used directly!
struct daxa_BufferId
{
    daxa_u32 value;
};
struct daxa_ImageViewId
{
    daxa_u32 value;
};
struct daxa_SamplerId
{
    daxa_u32 value;
};

// One can get the bindless table index from the id directly in the shader:
daxa_u32 daxa_id_to_index(daxa_BufferId id) { return (DAXA_ID_INDEX_MASK & id.value); }
daxa_u32 daxa_id_to_index(daxa_ImageViewId id) { return (DAXA_ID_INDEX_MASK & id.value); }
daxa_u32 daxa_id_to_index(daxa_SamplerId id) { return (DAXA_ID_INDEX_MASK & id.value); }
layout(scalar, binding = DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING, set = 0) readonly buffer daxa_BufferDeviceAddressBufferBlock { daxa_u64 addresses[]; }
daxa_buffer_device_address_buffer;
daxa_u64 daxa_id_to_address(daxa_BufferId buffer_id) { return daxa_buffer_device_address_buffer.addresses[daxa_id_to_index(buffer_id)]; }

// One can get a corresponding glsl object from the bindless tables easily with the following makros:
#define daxa_id_to_rwbuffer(NAME, buffer_id) daxa_id_to_rwbuffer##NAME(buffer_id)
#define daxa_id_to_buffer(NAME, buffer_id) daxa_id_to_buffer##NAME(buffer_id)
#define daxa_get_image(IMAGE_TYPE, image_id) daxa_RWImageTable##IMAGE_TYPE[daxa_id_to_index(image_id)]
#define daxa_get_texture(TEXTURE_TYPE, image_id) daxa_ImageTable##TEXTURE_TYPE[daxa_id_to_index(image_id)]
layout(binding = DAXA_SAMPLER_BINDING, set = 0) uniform sampler daxa_SamplerTable[];
#define daxa_get_sampler(sampler_id) daxa_SamplerTable[daxa_id_to_index(sampler_id)]

// Buffers and images have strongly typed handles:
// The image types have overloads for the most important glsl access functions.
#define deref(BUFFER_PTR) BUFFER_PTR.value
#define daxa_RWBufferPtr(STRUCT_TYPE) daxa_RWBufferPtr##STRUCT_TYPE
#define daxa_BufferPtr(STRUCT_TYPE) daxa_BufferPtr##STRUCT_TYPE
#define daxa_RWImage(DIMENSION, SCALAR_TYPE) daxa_RWImage##DIMENSION##SCALAR_TYPE
#define daxa_Image(DIMENSION, SCALAR_TYPE) daxa_Image##DIMENSION##SCALAR_TYPE

// One can declare their own table alias by using the predefined layours, or simply use the predefined binding points like here:
#define DAXA_BUFFER_REFERENCE_LAYOUT layout(buffer_reference, scalar, buffer_reference_align = 4)
#define DAXA_STORAGE_IMAGE_LAYOUT layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0)
#define DAXA_SAMPLED_IMAGE_LAYOUT layout(binding = DAXA_SAMPLED_IMAGE_BINDING, set = 0)
#define DAXA_SAMPLER_LAYOUT layout(binding = DAXA_SAMPLER_BINDING, set = 0)

#define DAXA_ENABLE_BUFFER_PTR(STRUCT_TYPE)                                                                             \
    DAXA_BUFFER_REFERENCE_LAYOUT buffer daxa_RWBufferPtr##STRUCT_TYPE                                                   \
    {                                                                                                                   \
        STRUCT_TYPE value;                                                                                              \
    };                                                                                                                  \
    DAXA_BUFFER_REFERENCE_LAYOUT readonly buffer daxa_BufferPtr##STRUCT_TYPE                                            \
    {                                                                                                                   \
        STRUCT_TYPE value;                                                                                              \
    };                                                                                                                  \
    daxa_RWBufferPtr##STRUCT_TYPE daxa_id_to_rwbuffer_ptr##STRUCT_TYPE(daxa_BufferId buffer_id)                         \
    {                                                                                                                   \
        return daxa_RWBufferPtr##STRUCT_TYPE(daxa_buffer_device_address_buffer.addresses[daxa_id_to_index(buffer_id)]); \
    }                                                                                                                   \
    daxa_BufferPtr##STRUCT_TYPE daxa_id_to_buffer_ptr##STRUCT_TYPE(daxa_BufferId buffer_id)                             \
    {                                                                                                                   \
        return daxa_BufferPtr##STRUCT_TYPE(daxa_buffer_device_address_buffer.addresses[daxa_id_to_index(buffer_id)]);   \
    }

// Use this to allow code sharing and daxa's buffer syntax sugar to work:
#define DAXA_DECL_BUFFER(NAME, BODY)                                                                          \
    DAXA_BUFFER_REFERENCE_LAYOUT buffer daxa_RWBuffer##NAME BODY;                                             \
    DAXA_BUFFER_REFERENCE_LAYOUT readonly buffer daxa_Buffer##NAME BODY;                                      \
    daxa_RWBuffer##NAME daxa_id_to_rwbuffer##NAME(daxa_BufferId buffer_id)                                    \
    {                                                                                                         \
        return daxa_RWBuffer##NAME(daxa_buffer_device_address_buffer.addresses[daxa_id_to_index(buffer_id)]); \
    }                                                                                                         \
    daxa_Buffer##NAME daxa_id_to_buffer##NAME(daxa_BufferId buffer_id)                                        \
    {                                                                                                         \
        return daxa_Buffer##NAME(daxa_buffer_device_address_buffer.addresses[daxa_id_to_index(buffer_id)]);   \
    }

// Use this to allow code sharing and daxa's buffer syntax sugar to work:
#define DAXA_DECL_BUFFER_STRUCT(NAME, BODY) \
    struct NAME BODY;                       \
    DAXA_DECL_BUFFER(NAME, BODY)

// Some extra syntax suggar:
#define DAXA_USE_PUSH_CONSTANT(NAME)                          \
    layout(push_constant, scalar) uniform _DAXA_PUSH_CONSTANT \
    {                                                         \
        NAME daxa_push_constant;                              \
    };

#if !defined(DAXA_ENABLE_IMAGE_OVERLOADS_BASIC)
#define DAXA_ENABLE_IMAGE_OVERLOADS_BASIC 0
#endif

#define _DAXA_DECL_IMAGE_KIND(GLSL_TEXTURE_NAME, IMAGE_KIND, SCALAR_TYPE)                     \
    DAXA_SAMPLED_IMAGE_LAYOUT uniform GLSL_TEXTURE_NAME daxa_ImageTable##GLSL_TEXTURE_NAME[]; \
    struct daxa_Image##IMAGE_KIND##SCALAR_TYPE                                                \
    {                                                                                         \
        daxa_ImageViewId id;                                                                  \
    };

#define _DAXA_DECL_RWIMAGE_KIND(GLSL_IMAGE_NAME, IMAGE_KIND, SCALAR_TYPE)                   \
    DAXA_STORAGE_IMAGE_LAYOUT uniform GLSL_IMAGE_NAME daxa_RWImageTable##GLSL_IMAGE_NAME[]; \
    struct daxa_RWImage##IMAGE_KIND##SCALAR_TYPE                                            \
    {                                                                                       \
        daxa_ImageViewId id;                                                                \
    };

#define _DAXA_DECL_IMAGE_TYPE(IMAGE_KIND)                          \
    _DAXA_DECL_IMAGE_KIND(texture##IMAGE_KIND, IMAGE_KIND, f32)    \
    _DAXA_DECL_IMAGE_KIND(itexture##IMAGE_KIND, IMAGE_KIND, i32)   \
    _DAXA_DECL_IMAGE_KIND(utexture##IMAGE_KIND, IMAGE_KIND, u32)   \
    _DAXA_DECL_RWIMAGE_KIND(image##IMAGE_KIND, IMAGE_KIND, f32)    \
    _DAXA_DECL_RWIMAGE_KIND(iimage##IMAGE_KIND, IMAGE_KIND, i32)   \
    _DAXA_DECL_RWIMAGE_KIND(uimage##IMAGE_KIND, IMAGE_KIND, u32)   \
    _DAXA_DECL_RWIMAGE_KIND(i64image##IMAGE_KIND, IMAGE_KIND, i64) \
    _DAXA_DECL_RWIMAGE_KIND(u64image##IMAGE_KIND, IMAGE_KIND, u64)

_DAXA_DECL_IMAGE_TYPE(1D)
_DAXA_DECL_IMAGE_TYPE(2D)
_DAXA_DECL_IMAGE_TYPE(3D)
_DAXA_DECL_IMAGE_TYPE(Cube)
_DAXA_DECL_IMAGE_TYPE(CubeArray)
_DAXA_DECL_IMAGE_TYPE(1DArray)
_DAXA_DECL_IMAGE_TYPE(2DArray)
_DAXA_DECL_IMAGE_TYPE(2DMS)
_DAXA_DECL_IMAGE_TYPE(2DMSArray)

#if DAXA_ENABLE_IMAGE_OVERLOADS_BASIC

#define _DAXA_SAMPLE_PARAM(x)

#define _DAXA_REGISTER_RWIMAGE_KIND(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION, SCALAR_TYPE)                                                     \
    daxa_##SCALAR_TYPE##vec4 imageLoad(daxa_RWImage##IMAGE_KIND##SCALAR_TYPE image, daxa_i32vec##INDEX_DIMENSION index _DAXA_SAMPLE_PARAM(daxa_i32 s))             \
    {                                                                                                                                                              \
        return imageLoad(daxa_get_image(GLSL_IMAGE_NAME, image.id), index _DAXA_SAMPLE_PARAM(s));                                                                  \
    }                                                                                                                                                              \
    void imageStore(daxa_RWImage##IMAGE_KIND##SCALAR_TYPE image, daxa_i32vec##INDEX_DIMENSION index _DAXA_SAMPLE_PARAM(daxa_i32 s), daxa_##SCALAR_TYPE##vec4 data) \
    {                                                                                                                                                              \
        imageStore(daxa_get_image(GLSL_IMAGE_NAME, image.id), index _DAXA_SAMPLE_PARAM(s), data);                                                                  \
    }                                                                                                                                                              \
    daxa_i32vec##SIZE_DIMENSION imageSize(daxa_RWImage##IMAGE_KIND##SCALAR_TYPE image)                                                                             \
    {                                                                                                                                                              \
        return imageSize(daxa_get_image(GLSL_IMAGE_NAME, image.id));                                                                                               \
    }

#define _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, DIMENSION, SCALAR_TYPE, OP)                                                                           \
    daxa_##SCALAR_TYPE imageAtomic##OP(daxa_RWImage##IMAGE_KIND##SCALAR_TYPE image, daxa_i32vec##DIMENSION index _DAXA_SAMPLE_PARAM(daxa_i32 s), daxa_##SCALAR_TYPE data) \
    {                                                                                                                                                                     \
        return imageAtomic##OP(daxa_get_image(GLSL_IMAGE_NAME, image.id), index _DAXA_SAMPLE_PARAM(s), data);                                                             \
    }

#define _DAXA_REGISTER_ATOMIC_IMAGE_KIND(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, ATOMIC_FORMAT_QUALIFIER)                                                                                    \
    layout(binding = DAXA_STORAGE_IMAGE_BINDING, set = 0, ATOMIC_FORMAT_QUALIFIER) uniform GLSL_IMAGE_NAME daxa_AtomicImageTable##GLSL_IMAGE_NAME[];                                                            \
    daxa_##SCALAR_TYPE imageAtomicCompSwap(daxa_RWImage##IMAGE_KIND##SCALAR_TYPE image, daxa_i32vec##INDEX_DIMENSION index _DAXA_SAMPLE_PARAM(daxa_i32 s), daxa_##SCALAR_TYPE compare, daxa_##SCALAR_TYPE data) \
    {                                                                                                                                                                                                           \
        return imageAtomicCompSwap(daxa_get_image(GLSL_IMAGE_NAME, image.id), index _DAXA_SAMPLE_PARAM(s), compare, data);                                                                                      \
    }                                                                                                                                                                                                           \
    _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, Exchange)                                                                                                         \
    _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, Add)                                                                                                              \
    _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, And)                                                                                                              \
    _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, Or)                                                                                                               \
    _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, Xor)                                                                                                              \
    _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, Min)                                                                                                              \
    _DAXA_REGISTER_IMAGE_ATOMIC_OP(GLSL_IMAGE_NAME, IMAGE_KIND, INDEX_DIMENSION, SCALAR_TYPE, Max)

#define _DAXA_REGISTER_RWIMAGE_TYPES(IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION)                     \
    _DAXA_REGISTER_RWIMAGE_KIND(image##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION, f32)  \
    _DAXA_REGISTER_RWIMAGE_KIND(iimage##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION, i32) \
    _DAXA_REGISTER_RWIMAGE_KIND(uimage##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION, u32)

#define _DAXA_REGISTER_RWIMAGE_TYPES_64BIT(IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION)                 \
    _DAXA_REGISTER_RWIMAGE_KIND(i64image##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION, i64) \
    _DAXA_REGISTER_RWIMAGE_KIND(u64image##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION, u64)

#define _DAXA_REGISTER_ATOMIC_IMAGE_TYPES(IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION)           \
    _DAXA_REGISTER_ATOMIC_IMAGE_KIND(iimage##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, i32, r32i) \
    _DAXA_REGISTER_ATOMIC_IMAGE_KIND(uimage##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, u32, r32ui)

#define _DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(IMAGE_KIND, INDEX_DIMENSION, SIZE_DIMENSION)       \
    _DAXA_REGISTER_ATOMIC_IMAGE_KIND(i64image##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, i64, r64i) \
    _DAXA_REGISTER_ATOMIC_IMAGE_KIND(u64image##IMAGE_KIND, IMAGE_KIND, INDEX_DIMENSION, u64, r64ui)

#if !defined(DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE)
#define DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE 0
#endif

_DAXA_REGISTER_RWIMAGE_TYPES(1D, 1, 1)
_DAXA_REGISTER_RWIMAGE_TYPES(2D, 2, 2)
_DAXA_REGISTER_RWIMAGE_TYPES(3D, 3, 3)
_DAXA_REGISTER_RWIMAGE_TYPES(Cube, 3, 2)
_DAXA_REGISTER_RWIMAGE_TYPES(CubeArray, 3, 3)
_DAXA_REGISTER_RWIMAGE_TYPES(1DArray, 2, 2)
_DAXA_REGISTER_RWIMAGE_TYPES(2DArray, 3, 3)
#if DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x) , x
_DAXA_REGISTER_RWIMAGE_TYPES(2DMS, 2, 2)
_DAXA_REGISTER_RWIMAGE_TYPES(2DMSArray, 3, 3)
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x)
#endif

#if !defined(DAXA_ENABLE_IMAGE_OVERLOADS_64BIT)
#define DAXA_ENABLE_IMAGE_OVERLOADS_64BIT 0
#endif
#if DAXA_ENABLE_IMAGE_OVERLOADS_64BIT
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(1D, 1, 1)
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(2D, 2, 2)
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(3D, 3, 3)
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(Cube, 3, 2)
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(CubeArray, 3, 3)
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(1DArray, 2, 2)
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(2DArray, 3, 3)
#if DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x) , x
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(2DMS, 2, 2)
_DAXA_REGISTER_RWIMAGE_TYPES_64BIT(2DMSArray, 3, 3)
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x)
#endif
#endif

#if !defined(DAXA_ENABLE_IMAGE_OVERLOADS_ATOMIC)
#define DAXA_ENABLE_IMAGE_OVERLOADS_ATOMIC 0
#endif
#if DAXA_ENABLE_IMAGE_OVERLOADS_ATOMIC
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(1D, 1, 1)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(2D, 2, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(3D, 3, 3)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(Cube, 3, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(CubeArray, 3, 3)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(1DArray, 2, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(2DArray, 3, 3)
#if DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x) , x
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(2DMS, 2, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES(2DMSArray, 3, 3)
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x)
#endif

#if DAXA_ENABLE_IMAGE_OVERLOADS_64BIT
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(1D, 1, 1)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(2D, 2, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(3D, 3, 3)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(Cube, 3, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(CubeArray, 3, 3)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(1DArray, 2, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(2DArray, 3, 3)
#if DAXA_ENABLE_IMAGE_OVERLOADS_MULTISAMPLE
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x) , x
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(2DMS, 2, 2)
_DAXA_REGISTER_ATOMIC_IMAGE_TYPES_64BIT(2DMSArray, 3, 3)
#undef _DAXA_SAMPLE_PARAM
#define _DAXA_SAMPLE_PARAM(x)
#endif
#endif
#endif

#define _DAXA_REGISTER_IMAGE_KIND(GLSL_TEXTURE_NAME, GLSL_SAMPLER_NAME, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, SCALAR_TYPE)                                                                   \
    daxa_##SCALAR_TYPE##vec4 texture(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_SamplerId sampler_id, daxa_f32vec##UV_DIMENSION uv)                                                                         \
    {                                                                                                                                                                                                            \
        return texture(                                                                                                                                                                                          \
            GLSL_SAMPLER_NAME(                                                                                                                                                                                   \
                daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                                                                                   \
                daxa_get_sampler(sampler_id)),                                                                                                                                                                   \
            uv);                                                                                                                                                                                                 \
    }                                                                                                                                                                                                            \
    daxa_##SCALAR_TYPE##vec4 textureLod(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_SamplerId sampler_id, daxa_f32vec##UV_DIMENSION uv, daxa_f32 bias)                                                       \
    {                                                                                                                                                                                                            \
        return textureLod(                                                                                                                                                                                       \
            GLSL_SAMPLER_NAME(                                                                                                                                                                                   \
                daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                                                                                   \
                daxa_get_sampler(sampler_id)),                                                                                                                                                                   \
            uv,                                                                                                                                                                                                  \
            bias);                                                                                                                                                                                               \
    }                                                                                                                                                                                                            \
    daxa_##SCALAR_TYPE##vec4 textureGrad(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_SamplerId sampler_id, daxa_f32vec##UV_DIMENSION uv, daxa_f32vec##GRAD_DIMENSION dTdx, daxa_f32vec##GRAD_DIMENSION dTdy) \
    {                                                                                                                                                                                                            \
        return textureGrad(                                                                                                                                                                                      \
            GLSL_SAMPLER_NAME(                                                                                                                                                                                   \
                daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                                                                                   \
                daxa_get_sampler(sampler_id)),                                                                                                                                                                   \
            uv,                                                                                                                                                                                                  \
            dTdx,                                                                                                                                                                                                \
            dTdy);                                                                                                                                                                                               \
    }                                                                                                                                                                                                            \
    daxa_i32##vec##SIZE_DIMENSION textureSize(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_i32 lod)                                                                                                           \
    {                                                                                                                                                                                                            \
        return textureSize(                                                                                                                                                                                      \
            daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                                                                                       \
            lod);                                                                                                                                                                                                \
    }                                                                                                                                                                                                            \
    daxa_i32 textureQueryLevels(daxa_Image##IMAGE_KIND##SCALAR_TYPE image)                                                                                                                                       \
    {                                                                                                                                                                                                            \
        return textureQueryLevels(daxa_get_texture(GLSL_TEXTURE_NAME, image.id));                                                                                                                                \
    }

#define _DAXA_REGISTER_IMAGE_KIND_GATHER(GLSL_TEXTURE_NAME, GLSL_SAMPLER_NAME, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, SCALAR_TYPE) \
    daxa_##SCALAR_TYPE##vec4 textureGatherX(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_SamplerId sampler_id, daxa_f32vec##UV_DIMENSION uv)       \
    {                                                                                                                                                 \
        return textureGather(                                                                                                                         \
            GLSL_SAMPLER_NAME(                                                                                                                        \
                daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                        \
                daxa_get_sampler(sampler_id)),                                                                                                        \
            uv,                                                                                                                                       \
            0);                                                                                                                                       \
    }                                                                                                                                                 \
    daxa_##SCALAR_TYPE##vec4 textureGatherY(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_SamplerId sampler_id, daxa_f32vec##UV_DIMENSION uv)       \
    {                                                                                                                                                 \
        return textureGather(                                                                                                                         \
            GLSL_SAMPLER_NAME(                                                                                                                        \
                daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                        \
                daxa_get_sampler(sampler_id)),                                                                                                        \
            uv,                                                                                                                                       \
            1);                                                                                                                                       \
    }                                                                                                                                                 \
    daxa_##SCALAR_TYPE##vec4 textureGatherZ(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_SamplerId sampler_id, daxa_f32vec##UV_DIMENSION uv)       \
    {                                                                                                                                                 \
        return textureGather(                                                                                                                         \
            GLSL_SAMPLER_NAME(                                                                                                                        \
                daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                        \
                daxa_get_sampler(sampler_id)),                                                                                                        \
            uv,                                                                                                                                       \
            2);                                                                                                                                       \
    }                                                                                                                                                 \
    daxa_##SCALAR_TYPE##vec4 textureGatherW(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_SamplerId sampler_id, daxa_f32vec##UV_DIMENSION uv)       \
    {                                                                                                                                                 \
        return textureGather(                                                                                                                         \
            GLSL_SAMPLER_NAME(                                                                                                                        \
                daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                        \
                daxa_get_sampler(sampler_id)),                                                                                                        \
            uv,                                                                                                                                       \
            3);                                                                                                                                       \
    }

#define _DAXA_REGISTER_IMAGE_KIND_TEXEL_FETCH(GLSL_TEXTURE_NAME, GLSL_SAMPLER_NAME, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, SCALAR_TYPE) \
    daxa_##SCALAR_TYPE##vec4 texelFetch(daxa_Image##IMAGE_KIND##SCALAR_TYPE image, daxa_i32vec##UV_DIMENSION index, daxa_i32 sample_or_lod)                \
    {                                                                                                                                                      \
        return texelFetch(                                                                                                                                 \
            daxa_get_texture(GLSL_TEXTURE_NAME, image.id),                                                                                                 \
            index,                                                                                                                                         \
            sample_or_lod);                                                                                                                                \
    }

#define _DAXA_REGISTER_IMAGE_TYPES(IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION)                                             \
    _DAXA_REGISTER_IMAGE_KIND(texture##IMAGE_KIND, sampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, f32)   \
    _DAXA_REGISTER_IMAGE_KIND(itexture##IMAGE_KIND, isampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, i32) \
    _DAXA_REGISTER_IMAGE_KIND(utexture##IMAGE_KIND, usampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, u32)

#define _DAXA_REGISTER_IMAGE_TYPES_GATHER(IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION)                                             \
    _DAXA_REGISTER_IMAGE_KIND_GATHER(texture##IMAGE_KIND, sampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, f32)   \
    _DAXA_REGISTER_IMAGE_KIND_GATHER(itexture##IMAGE_KIND, isampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, i32) \
    _DAXA_REGISTER_IMAGE_KIND_GATHER(utexture##IMAGE_KIND, usampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, u32)

#define _DAXA_REGISTER_IMAGE_TYPES_TEXEL_FETCH(IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION)                                             \
    _DAXA_REGISTER_IMAGE_KIND_TEXEL_FETCH(texture##IMAGE_KIND, sampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, f32)   \
    _DAXA_REGISTER_IMAGE_KIND_TEXEL_FETCH(itexture##IMAGE_KIND, isampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, i32) \
    _DAXA_REGISTER_IMAGE_KIND_TEXEL_FETCH(utexture##IMAGE_KIND, usampler##IMAGE_KIND, IMAGE_KIND, UV_DIMENSION, GRAD_DIMENSION, SIZE_DIMENSION, u32)

_DAXA_REGISTER_IMAGE_TYPES(1D, 1, 1, 1)
_DAXA_REGISTER_IMAGE_TYPES(2D, 2, 2, 2)
_DAXA_REGISTER_IMAGE_TYPES(3D, 3, 3, 3)
_DAXA_REGISTER_IMAGE_TYPES(Cube, 3, 3, 2)
_DAXA_REGISTER_IMAGE_TYPES(CubeArray, 4, 3, 3)
_DAXA_REGISTER_IMAGE_TYPES(1DArray, 2, 1, 2)
_DAXA_REGISTER_IMAGE_TYPES(2DArray, 3, 2, 3)

_DAXA_REGISTER_IMAGE_TYPES_TEXEL_FETCH(1D, 1, 1, 1)
_DAXA_REGISTER_IMAGE_TYPES_TEXEL_FETCH(2D, 2, 2, 2)
_DAXA_REGISTER_IMAGE_TYPES_TEXEL_FETCH(3D, 3, 3, 3)
_DAXA_REGISTER_IMAGE_TYPES_TEXEL_FETCH(1DArray, 2, 1, 2)
_DAXA_REGISTER_IMAGE_TYPES_TEXEL_FETCH(2DArray, 3, 2, 3)

_DAXA_REGISTER_IMAGE_TYPES_GATHER(2D, 2, 2, 2)
_DAXA_REGISTER_IMAGE_TYPES_GATHER(Cube, 3, 3, 2)
_DAXA_REGISTER_IMAGE_TYPES_GATHER(CubeArray, 4, 3, 3)
_DAXA_REGISTER_IMAGE_TYPES_GATHER(2DArray, 3, 2, 3)

#endif

#if !defined(DAXA_ENABLE_SHADER_NO_NAMESPACE)
#define DAXA_ENABLE_SHADER_NO_NAMESPACE 0
#endif
#if DAXA_ENABLE_SHADER_NO_NAMESPACE
#if !defined(DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES)
#define DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES 1
#endif

#define BUFFER_REFERENCE_LAYOUT DAXA_BUFFER_REFERENCE_LAYOUT
#define STORAGE_IMAGE_LAYOUT DAXA_STORAGE_IMAGE_LAYOUT
#define SAMPLED_IMAGE_LAYOUT DAXA_SAMPLED_IMAGE_LAYOUT
#define SAMPLER_LAYOUT DAXA_SAMPLER_LAYOUT

#define BufferId daxa_BufferId
#define ImageViewId daxa_ImageViewId
#define SamplerId daxa_SamplerId

#define RWBufferPtr daxa_RWBufferPtr
#define BufferPtr daxa_BufferPtr
#define RWImage daxa_RWImage
#define Image daxa_RWImage

#define get_image daxa_get_image
#define get_texture daxa_get_texture
#define get_sampler daxa_get_sampler

#define id_to_address daxa_id_to_address
#define id_to_index daxa_id_to_index

#endif

#if !defined(DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES)
#define DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES 0
#endif
#if DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES
#define b32 daxa_b32
#define f32 daxa_f32
#define i32 daxa_i32
#define u32 daxa_u32
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
#define i64vec2 daxa_i64vec2
#define i64vec3 daxa_i64vec3
#define i64vec4 daxa_i64vec4
#define u64 daxa_u64
#define u64vec1 daxa_u64vec1
#define u64vec2 daxa_u64vec2
#define u64vec3 daxa_u64vec3
#define u64vec4 daxa_u64vec4
#endif

DAXA_ENABLE_BUFFER_PTR(daxa_b32)
DAXA_ENABLE_BUFFER_PTR(daxa_f32)
DAXA_ENABLE_BUFFER_PTR(daxa_i32)
DAXA_ENABLE_BUFFER_PTR(daxa_u32)
DAXA_ENABLE_BUFFER_PTR(daxa_b32vec1)
DAXA_ENABLE_BUFFER_PTR(daxa_b32vec2)
DAXA_ENABLE_BUFFER_PTR(daxa_b32vec3)
DAXA_ENABLE_BUFFER_PTR(daxa_b32vec4)
DAXA_ENABLE_BUFFER_PTR(daxa_f32)
DAXA_ENABLE_BUFFER_PTR(daxa_f32vec1)
DAXA_ENABLE_BUFFER_PTR(daxa_f32vec2)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat2x2)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat2x3)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat2x4)
DAXA_ENABLE_BUFFER_PTR(daxa_f32vec3)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat3x2)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat3x3)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat3x4)
DAXA_ENABLE_BUFFER_PTR(daxa_f32vec4)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat4x2)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat4x3)
DAXA_ENABLE_BUFFER_PTR(daxa_f32mat4x4)
DAXA_ENABLE_BUFFER_PTR(daxa_i32)
DAXA_ENABLE_BUFFER_PTR(daxa_i32vec1)
DAXA_ENABLE_BUFFER_PTR(daxa_i32vec2)
DAXA_ENABLE_BUFFER_PTR(daxa_i32vec3)
DAXA_ENABLE_BUFFER_PTR(daxa_i32vec4)
DAXA_ENABLE_BUFFER_PTR(daxa_u32)
DAXA_ENABLE_BUFFER_PTR(daxa_u32vec1)
DAXA_ENABLE_BUFFER_PTR(daxa_u32vec2)
DAXA_ENABLE_BUFFER_PTR(daxa_u32vec3)
DAXA_ENABLE_BUFFER_PTR(daxa_u32vec4)
DAXA_ENABLE_BUFFER_PTR(daxa_i64)
DAXA_ENABLE_BUFFER_PTR(daxa_i64vec1)
DAXA_ENABLE_BUFFER_PTR(daxa_i64vec2)
DAXA_ENABLE_BUFFER_PTR(daxa_i64vec3)
DAXA_ENABLE_BUFFER_PTR(daxa_i64vec4)
DAXA_ENABLE_BUFFER_PTR(daxa_u64)
DAXA_ENABLE_BUFFER_PTR(daxa_u64vec1)
DAXA_ENABLE_BUFFER_PTR(daxa_u64vec2)
DAXA_ENABLE_BUFFER_PTR(daxa_u64vec3)
DAXA_ENABLE_BUFFER_PTR(daxa_u64vec4)