#ifndef __DAXA_CORE_H__
#define __DAXA_CORE_H__

#include "vulkan.h"
#include "stdint.h"

typedef enum
{
    DAXA_RESULT_UNKNOWN = 0,
    DAXA_RESULT_OUT_OF_MEMORY = 1,
    DAXA_RESULT_MAX_ENUM = 0xFFFFFFFF,
} daxa_Result;

#define DAXA_BOOL uint32_t

#endif // #ifndef __DAXA_CORE_H__