#pragma once
#include "../daxa.inl"

#if DAXA_SHADER
#define DAXA_INL_TASK_USE_BEGIN(NAME, SLOT) \
    DAXA_CONSTANT_BUFFER(SLOT)              \
    NAME                                    \
    {
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS) TYPE NAME;
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) TYPE NAME;
#define DAXA_INL_TASK_USE_END() \
    }                           \
    ;
#else
#include "task_list_types.hpp"
#define DAXA_INL_TASK_USE_BEGIN(TASK_NAME, SLOT)             \
    struct TASK_NAME                                         \
    {                                                        \
        static constexpr isize CONSANT_BUFFER_SLOT = SLOT;   \
        static constexpr std::string_view NAME = #TASK_NAME; \
        std::string name = #TASK_NAME;                       \
        struct Uses                                          \
        {
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) daxa::TaskBufferUse<daxa::TaskBufferAccess::TASK_ACCESS> NAME{};
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS) daxa::TaskImageUse<daxa::TaskImageAccess::TASK_ACCESS, TYPE::view_type()> NAME{};
#define DAXA_INL_TASK_USE_END() \
    }                           \
    uses = {};                  \
    }                           \
    ;

#endif