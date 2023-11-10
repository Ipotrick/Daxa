#pragma once

#define DAXA_UNIFORM_BUFFER_SLOT0 0
#define DAXA_UNIFORM_BUFFER_SLOT1 1
#define DAXA_UNIFORM_BUFFER_SLOT2 2
#define DAXA_UNIFORM_BUFFER_SLOT3 3
#define DAXA_UNIFORM_BUFFER_SLOT4 4
#define DAXA_UNIFORM_BUFFER_SLOT5 5
#define DAXA_UNIFORM_BUFFER_SLOT6 6
#define DAXA_UNIFORM_BUFFER_SLOT7 7

#define DAXA_GPU_TABLE_SET_BINDING 0
#define DAXA_DECL_UNIFORM_BUFFER_BINDING_SET 1
#define DAXA_STORAGE_BUFFER_BINDING 0
#define DAXA_STORAGE_IMAGE_BINDING 1
#define DAXA_SAMPLED_IMAGE_BINDING 2
#define DAXA_SAMPLER_BINDING 3
#define DAXA_BUFFER_DEVICE_ADDRESS_BUFFER_BINDING 4
#define DAXA_ACCELERATION_STRUCTURE_BINDING 5

#if defined(_STDC_) // C
#define DAXA_SHADER 0
#elif defined(__cplusplus) // C++
#define DAXA_SHADER 0
#elif defined(GL_core_profile) // GLSL
#define DAXA_SHADER 1
#define DAXA_SHADERLANG DAXA_SHADERLANG_GLSL
#elif defined(__HLSL_VERSION) // HLSL
#define DAXA_SHADER 1
#define DAXA_SHADERLANG DAXA_SHADERLANG_HLSL
#endif

#if DAXA_SHADER
#define DAXA_SHADERLANG_GLSL 1
#define DAXA_SHADERLANG_HLSL 2
#if DAXA_SHADERLANG == DAXA_SHADERLANG_GLSL
#include <daxa/daxa.glsl>
#elif DAXA_SHADERLANG == DAXA_SHADERLANG_HLSL
#include <daxa/daxa.hlsl>
#endif
#else
#if defined(_STDC_)
#include <daxa/c/daxa.h>

#if !defined(DAXA_UNIFORM_BUFFER_ALIGNMENT)
#define DAXA_UNIFORM_BUFFER_ALIGNMENT 64
#endif // #if !defined(DAXA_UNIFORM_BUFFER_ALIGNMENT)

/// @brief The c++ equivalent of a constant buffer in a file is simply a struct.
#define DAXA_DECL_UNIFORM_BUFFER(SLOT) typedef struct alignas(DAXA_UNIFORM_BUFFER_ALIGNMENT)
/// @brief Buffer ptr enable is ignored in c++.
#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN)
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_RWBufferPtr(x) daxa_BufferDeviceAddress
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_BufferPtr(x) daxa_BufferDeviceAddress

#elif defined(__cplusplus)
#include <daxa/daxa.hpp>

#if !defined(DAXA_UNIFORM_BUFFER_ALIGNMENT)
#define DAXA_UNIFORM_BUFFER_ALIGNMENT 64
#endif // #if !defined(DAXA_UNIFORM_BUFFER_ALIGNMENT)

/// @brief The c++ equivalent of a constant buffer in a file is simply a struct.
#define DAXA_DECL_UNIFORM_BUFFER(SLOT) struct alignas(DAXA_UNIFORM_BUFFER_ALIGNMENT)
/// @brief Buffer ptr enable is ignored in c++.
#define DAXA_DECL_BUFFER_PTR(STRUCT_TYPE)
#define DAXA_DECL_BUFFER_PTR_ALIGN(STRUCT_TYPE, ALIGN)
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_RWBufferPtr(x) daxa::types::BufferDeviceAddress
/// @brief Buffer ptr types map to the buffer device address type in daxa.
#define daxa_BufferPtr(x) daxa::types::BufferDeviceAddress

#endif
#endif
