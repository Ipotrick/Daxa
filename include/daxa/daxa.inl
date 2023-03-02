#pragma once

#if defined(DAXA_SHADER)
#if !defined(DAXA_STORAGE_BUFFER_BINDING)
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_SHADER_DEBUG_BUFFER_BINDING 5
#define DAXA_ID_INDEX_MASK (0x00FFFFFF)
#define DAXA_ID_VERSION_SHIFT (24)
#define DAXA_DEBUG_MESSAGE_MAX_COUNT 1024
#define DAXA_DEBUG_MESSAGE_SIZE 4
#define DAXA_DEBUG_MESSAGE_BUFFER_REPORT 1
#define DAXA_DEBUG_MESSAGE_IMAGE_REPORT 2
#define DAXA_DEBUG_MESSAGE_SAMPLER_REPORT 4
#endif
#define DAXA_SHADERLANG_GLSL 1
#define DAXA_SHADERLANG_HLSL 2
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
#include <daxa/daxa.glsl>
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
#include <daxa/daxa.hlsl>
#endif
#elif defined(__cplusplus)
#include <daxa/daxa.hpp>
#define _DAXA_REGISTER_TEXTURE_TYPE(IMAGE_TYPE)
#define _DAXA_REGISTER_IMAGE_TYPE(IMAGE_TYPE)                         \
    using daxa_##RWImage##IMAGE_TYPE##f32 = daxa::types::ImageViewId; \
    using daxa_##RWImage##IMAGE_TYPE##i32 = daxa::types::ImageViewId; \
    using daxa_##RWImage##IMAGE_TYPE##u32 = daxa::types::ImageViewId; \
    using daxa_##RWImage##IMAGE_TYPE##i64 = daxa::types::ImageViewId; \
    using daxa_##RWImage##IMAGE_TYPE##u64 = daxa::types::ImageViewId; \
    using daxa_##Image##IMAGE_TYPE##f32 = daxa::types::ImageViewId;   \
    using daxa_##Image##IMAGE_TYPE##i32 = daxa::types::ImageViewId;   \
    using daxa_##Image##IMAGE_TYPE##u32 = daxa::types::ImageViewId;   \
    using daxa_##Image##IMAGE_TYPE##i64 = daxa::types::ImageViewId;   \
    using daxa_##Image##IMAGE_TYPE##u64 = daxa::types::ImageViewId;
#define _DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE)
#define DAXA_PUSH_CONSTANT(STRUCT_TYPE)
#define DAXA_ENABLE_BUFFER_PTR(STRUCT_TYPE)
#define daxa_RWBufferPtr(x) daxa::types::BufferDeviceAddress
#define daxa_BufferPtr(x) daxa::types::BufferDeviceAddress

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

using daxa_b32 = daxa::types::b32;
using daxa_i32 = daxa::types::i32;
using daxa_u32 = daxa::types::u32;
using daxa_f32 = daxa::types::f32;
using daxa_b32vec2 = daxa::types::b32vec2;
using daxa_b32vec3 = daxa::types::b32vec3;
using daxa_b32vec4 = daxa::types::b32vec4;
using daxa_f32 = daxa::types::f32;
using daxa_f32vec2 = daxa::types::f32vec2;
using daxa_f32mat2x2 = daxa::types::f32mat2x2;
using daxa_f32mat2x3 = daxa::types::f32mat2x3;
using daxa_f32mat2x4 = daxa::types::f32mat2x4;
using daxa_f32vec3 = daxa::types::f32vec3;
using daxa_f32mat3x2 = daxa::types::f32mat3x2;
using daxa_f32mat3x3 = daxa::types::f32mat3x3;
using daxa_f32mat3x4 = daxa::types::f32mat3x4;
using daxa_f32vec4 = daxa::types::f32vec4;
using daxa_f32mat4x2 = daxa::types::f32mat4x2;
using daxa_f32mat4x3 = daxa::types::f32mat4x3;
using daxa_f32mat4x4 = daxa::types::f32mat4x4;
using daxa_i32 = daxa::types::i32;
using daxa_u32 = daxa::types::u32;
using daxa_i64 = daxa::types::i64;
using daxa_u64 = daxa::types::u64;
using daxa_i32vec2 = daxa::types::i32vec2;
using daxa_u32vec2 = daxa::types::u32vec2;
using daxa_i32vec3 = daxa::types::i32vec3;
using daxa_u32vec3 = daxa::types::u32vec3;
using daxa_i32vec4 = daxa::types::i32vec4;
using daxa_u32vec4 = daxa::types::u32vec4;
using daxa_BufferId = daxa::types::BufferId;
using daxa_ImageViewId = daxa::types::ImageViewId;
using daxa_ImageId = daxa::types::ImageId;
using daxa_SamplerId = daxa::types::SamplerId;

#if DAXA_ENABLE_SHADER_NO_NAMESPACE_PRIMITIVES
using namespace daxa::types;
#endif

#if !defined(DAXA_SHADER_GPU_ID_VALIDATION)
#define DAXA_SHADER_GPU_ID_VALIDATION (0)
#endif // #if !defined(DAXA_SHADER_GPU_ID_VALIDATION)

#endif
