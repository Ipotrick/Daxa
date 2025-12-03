#ifndef DAXA_DAXA_INL
#define DAXA_DAXA_INL

#define DAXA_GPU_TABLE_SET_BINDING 0
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ACCELERATION_STRUCTURE_BINDING 5

#define DAXA_LANGUAGE_C 0
#define DAXA_LANGUAGE_CPP 0
#define DAXA_LANGUAGE_GLSL 2
#define DAXA_LANGUAGE_SLANG 3
#define DAXA_LANGUAGE_HLSL DAXA_LANGUAGE_SLANG

#if defined(_STDC_) // C

#define DAXA_SHADER 0
#define DAXA_LANGUAGE DAXA_LANGUAGE_C
#include <daxa/c/daxa.h>
/// @brief Buffer ptr enable is ignored in c++.
#define DAXA_FWD_DECL_BUFFER_PTR(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN)
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_RWBufferPtr(x) daxa_DeviceAddress
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_BufferPtr(x) daxa_DeviceAddress

#elif defined(__cplusplus) // C++

#define DAXA_SHADER 0
#define DAXA_LANGUAGE DAXA_LANGUAGE_CPP
#include <daxa/daxa.hpp>
/// @brief Buffer ptr enable is ignored in c++.
#define DAXA_FWD_DECL_BUFFER_PTR(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN)
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_RWBufferPtr(x) daxa::types::DeviceAddress
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_BufferPtr(x) daxa::types::DeviceAddress

#elif defined(GL_core_profile) // GLSL

#define DAXA_SHADER 1
#define DAXA_LANGUAGE DAXA_LANGUAGE_GLSL
#include "daxa/daxa.glsl"

#else // SLANG

#define DAXA_SHADER 1
#define DAXA_LANGUAGE DAXA_LANGUAGE_SLANG
#include "daxa/daxa.slang"

#endif
#endif
