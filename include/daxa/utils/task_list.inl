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
// MASSIVE C++ COPE:
namespace daxa::detail
{
    template <usize SLOT, typename StructT>
    using TaskUsesSwitchedTemplateArgs = daxa::TaskUses<StructT, SLOT>;
}
#define DAXA_INL_TASK_USE_BEGIN(NAME, SLOT) \
    using NAME = daxa::detail::TaskUsesSwitchedTemplateArgs<SLOT, decltype([]() { struct T{
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) daxa::TaskBufferUse NAME = {{.access = daxa::TaskBufferAccess::TASK_ACCESS}};
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) daxa::TaskImageUse NAME = {{.access = daxa::TaskImageAccess::TASK_ACCESS, .slice = SLICE, .view_type = TYPE::view_type()}};
#define DAXA_INL_TASK_USE_END() \
    }                           \
    ;                           \
    return T{};                 \
    }())>;

#endif
