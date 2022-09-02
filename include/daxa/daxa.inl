#pragma once

#ifdef __cplusplus
#include <daxa/daxa.hpp>
#define DAXA_REGISTER_STRUCT_GET_BUFFER(STRUCT_TYPE)
#define DAXA_REGISTER_READ_ONLY_IMAGE_TYPE(IMAGE_TYPE)
#define DAXA_REGISTER_SAMPLER_TYPE(SAMPLER_TYPE)
#define DAXA_PUSH_CONSTANT(STRUCT_TYPE)
using namespace daxa::types;
#else
#include <daxa/daxa.glsl>
#endif
