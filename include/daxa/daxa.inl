#pragma once

#ifdef DAXA_SHADER_NO_NAMESPACE
#define DAXA_SHADER_NO_NAMESPACE_PRIMITIVES

#define BufferRef daxa_BufferRef
#define WrappedBufferRef daxa_WrappedBufferRef
#define CoherentBufferRef daxa_CoherentBufferRef
#define WrappedCoherentBufferRef daxa_WrappedCoherentBufferRef
#endif

#if defined(DAXA_SHADER)
#define DAXA_SHADERLANG_GLSL 1
#define DAXA_SHADERLANG_HLSL 2
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ID_INDEX_MASK (0x00FFFFFF)
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
#include <daxa/daxa.glsl>
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
#include "daxa/daxa.hlsl"
#endif
#elif defined(__cplusplus)
#include <daxa/daxa.hpp>
#define DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(IMAGE_TYPE)
#define DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE)
#define DAXA_PUSH_CONSTANT(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_STRUCT(NAME, BODY) \
    struct NAME BODY
#define DAXA_DECL_BUFFER(NAME, BODY)
#define daxa_BufferRef(x) daxa::types::BufferDeviceAddress
#define daxa_ROBufferRef(x) daxa::types::BufferDeviceAddress

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

#if defined(DAXA_SHADER_NO_NAMESPACE_PRIMITIVES)
using namespace daxa::types;
#endif
#endif
