#pragma once

#define DAXA_CBUFFER_SLOT0 0
#define DAXA_CBUFFER_SLOT1 1
#define DAXA_CBUFFER_SLOT2 2
#define DAXA_CBUFFER_SLOT3 3
#define DAXA_CBUFFER_SLOT4 4
#define DAXA_CBUFFER_SLOT5 5
#define DAXA_CBUFFER_SLOT6 6
#define DAXA_CBUFFER_SLOT7 7

#if defined(DAXA_SHADER)
#if !defined(DAXA_STORAGE_BUFFER_BINDING)
#define DAXA_GPU_TABLE_SET_BINDING 0
#define DAXA_CONSTANT_BUFFER_BINDING_SET 1
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_SHADER_DEBUG_BUFFER_BINDING 5
#define DAXA_ID_INDEX_MASK (0x00FFFFFF)
#define DAXA_ID_VERSION_SHIFT (24)
#endif
#define DAXA_SHADERLANG_GLSL 1
#define DAXA_SHADERLANG_HLSL 2

#if !defined(DAXA_ENABLE_SHADER_NO_NAMESPACE)
#define DAXA_ENABLE_SHADER_NO_NAMESPACE 0
#else
#if !defined(DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES)
#define DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES 1
#endif
#endif
#if !defined(DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES)
#define DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES 0
#endif

#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
#include <daxa/daxa.glsl>
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
#include <daxa/daxa.hlsl>
#endif
#elif defined(__cplusplus)
#include <daxa/daxa.hpp>
#define DAXA_CONSTANT_BUFFER(SLOT) struct alignas(64)
#define _DAXA_REGISTER_TEXTURE_TYPE(IMAGE_TYPE)
#define _DAXA_REGISTER_IMAGE_TYPE(IMAGE_TYPE)                                                               \
    using daxa_##RWImage##IMAGE_TYPE##f32 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>; \
    using daxa_##RWImage##IMAGE_TYPE##i32 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>; \
    using daxa_##RWImage##IMAGE_TYPE##u32 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>; \
    using daxa_##RWImage##IMAGE_TYPE##i64 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>; \
    using daxa_##RWImage##IMAGE_TYPE##u64 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>; \
    using daxa_##Image##IMAGE_TYPE##f32 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>;   \
    using daxa_##Image##IMAGE_TYPE##i32 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>;   \
    using daxa_##Image##IMAGE_TYPE##u32 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>;   \
    using daxa_##Image##IMAGE_TYPE##i64 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>;   \
    using daxa_##Image##IMAGE_TYPE##u64 = daxa::types::TypedImageViewId<daxa::_ShaderAlias_##IMAGE_TYPE>;
#define _DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE)
#define DAXA_PUSH_CONSTANT(STRUCT_TYPE)
#define DAXA_ENABLE_BUFFER_PTR(STRUCT_TYPE)
#define daxa_RWBufferPtr(x) alignas(8) daxa::types::BufferDeviceAddress
#define daxa_BufferPtr(x) alignas(8) daxa::types::BufferDeviceAddress

// NEEDED FOR HLSL SUPPORT
#define DAXA_DECL_BUFFER_STRUCT(Type, Body) struct Type Body;

_DAXA_REGISTER_IMAGE_TYPE(1D)
_DAXA_REGISTER_IMAGE_TYPE(2D)
_DAXA_REGISTER_IMAGE_TYPE(3D)
_DAXA_REGISTER_IMAGE_TYPE(1DArray)
_DAXA_REGISTER_IMAGE_TYPE(2DArray)
_DAXA_REGISTER_IMAGE_TYPE(Cube)
_DAXA_REGISTER_IMAGE_TYPE(CubeArray)
_DAXA_REGISTER_IMAGE_TYPE(2DMS)
_DAXA_REGISTER_IMAGE_TYPE(2DMSArray)

#define daxa_Image(DIM, SCALAR) daxa::types::ImageViewId
#define daxa_RWImage(DIM, SCALAR) daxa::types::ImageViewId

#define daxa_b32 daxa::types::b32
#define daxa_i32 daxa::types::i32
#define daxa_u32 daxa::types::u32
#define daxa_f32 daxa::types::f32
#define daxa_b32vec2 daxa::types::b32vec2
#define daxa_b32vec3 daxa::types::b32vec3
#define daxa_b32vec4 daxa::types::b32vec4
#define daxa_f32 daxa::types::f32
#define daxa_f32vec2 daxa::types::f32vec2
#define daxa_f32mat2x2 daxa::types::f32mat2x2
#define daxa_f32mat2x3 daxa::types::f32mat2x3
#define daxa_f32mat2x4 daxa::types::f32mat2x4
#define daxa_f32vec3 daxa::types::f32vec3
#define daxa_f32mat3x2 daxa::types::f32mat3x2
#define daxa_f32mat3x3 daxa::types::f32mat3x3
#define daxa_f32mat3x4 daxa::types::f32mat3x4
#define daxa_f32vec4 daxa::types::f32vec4
#define daxa_f32mat4x2 daxa::types::f32mat4x2
#define daxa_f32mat4x3 daxa::types::f32mat4x3
#define daxa_f32mat4x4 daxa::types::f32mat4x4
#define daxa_i32 daxa::types::i32
#define daxa_u32 daxa::types::u32
#define daxa_i64 daxa::types::i64
#define daxa_u64 daxa::types::u64
#define daxa_i32vec2 daxa::types::i32vec2
#define daxa_u32vec2 daxa::types::u32vec2
#define daxa_i32vec3 daxa::types::i32vec3
#define daxa_u32vec3 daxa::types::u32vec3
#define daxa_i32vec4 daxa::types::i32vec4
#define daxa_u32vec4 daxa::types::u32vec4
#define daxa_BufferId daxa::types::BufferId
#define daxa_ImageViewId daxa::types::ImageViewId
#define daxa_SamplerId daxa::types::SamplerId

#if DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES
using namespace daxa::types;
#endif

#endif
