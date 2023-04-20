#pragma once
#include "../daxa.inl"

#if DAXA_SHADER
#define DAXA_INL_TASK_USE_BEGIN(NAME, SLOT) \
    DAXA_CONSTANT_BUFFER(SLOT)              \
    NAME                                    \
    {
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) TYPE NAME;
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) TYPE NAME;
#define DAXA_INL_TASK_USE_END() \
    }                           \
    ;
#else
#include "task_list_types.hpp"
#define DAXA_INL_TASK_USE_BEGIN(NAME, SLOT)                         \
    using NAME = daxa::TaskUses<struct NAME##Struct, SLOT>; \
    struct NAME##Struct                                             \
    {
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) daxa::TaskBufferUse NAME = {{.access = daxa::TaskBufferAccess::TASK_ACCESS}};
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) daxa::TaskImageUse NAME = {{.access = daxa::TaskImageAccess::TASK_ACCESS, .slice = SLICE, .view_type = TYPE::view_type()}};
#define DAXA_INL_TASK_USE_END() \
    }                           \
    ;

#endif
