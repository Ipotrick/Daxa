#pragma once

#if defined(DAXA_SHADER)
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ID_INDEX_MASK (0x00FFFFFF)
#define DAXA_SHARED_TYPE(x) x
#if defined(DAXA_GLSL)
#include <daxa/daxa.glsl>
#elif defined(DAXA_HLSL)
#include "daxa/daxa.hlsl"
#endif
#elif defined(__cplusplus)
#include <daxa/daxa.hpp>
#define DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(IMAGE_TYPE)
#define DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE)
#define DAXA_PUSH_CONSTANT(STRUCT_TYPE)
#define DAXA_SHARED_TYPE(x) daxa::types::x
#define DAXA_DECL_BUFFER_STRUCT(NAME, BODY)     \
    using NAME##BufferRef = u64;                \
    using NAME##WrappedBufferRef = u64;         \
    using NAME##CoherentBufferRef = u64;        \
    using NAME##CoherentWrappedBufferRef = u64; \
    struct NAME BODY
#define BufferRef(x) u64
#define WrappedBufferRef(x) u64
#define CoherentBufferRef(x) u64
#define WrappedCoherentBufferRef(x) u64
#endif
