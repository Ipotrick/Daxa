#pragma once

#if defined(_DAXA_SHADER)
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_ID_INDEX_MASK (0x00FFFFFF)
#define DAXA_SHARED_TYPE(x) x
#endif

#if defined(_DAXA_GLSL)
#include <daxa/daxa.glsl>
#elif defined(_DAXA_HLSL)
#include "daxa/daxa.hlsl"
#elif defined(__cplusplus)
#include <daxa/daxa.hpp>
#define DAXA_REGISTER_STRUCT_GET_BUFFER(STRUCT_TYPE)
#define DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(IMAGE_TYPE)
#define DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE)
#define DAXA_PUSH_CONSTANT(STRUCT_TYPE)
#define DAXA_SHARED_TYPE(x) daxa::types::x

#define DAXA_DEFINE_GET_STRUCTURED_BUFFER(STRUCT_TYPE)
#endif
