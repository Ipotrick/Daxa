#pragma once
#include "../daxa.inl"

#if DAXA_SHADER
#define DAXA_DECL_TASK_USES_BEGIN(NAME, SLOT) \
    DAXA_DECL_UNIFORM_BUFFER(SLOT)            \
    NAME                                      \
    {
#define DAXA_TASK_USE_IMAGE(NAME, VIEW_TYPE, TASK_ACCESS) daxa_ImageViewId NAME;
#define DAXA_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) TYPE NAME;
#define DAXA_DECL_TASK_USES_END() \
    }                             \
    ;
#else
#include "task_graph_types.hpp"
#define DAXA_DECL_TASK_USES_BEGIN(TASK_NAME, SLOT)           \
    struct TASK_NAME                                         \
    {                                                        \
        static constexpr isize CONSANT_BUFFER_SLOT = SLOT;   \
        static constexpr std::string_view NAME = #TASK_NAME; \
        std::string name = #TASK_NAME;                       \
        struct Uses                                          \
        {
#define DAXA_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) daxa::TaskBufferUse<daxa::TaskBufferAccess::TASK_ACCESS> NAME{};
#define DAXA_TASK_USE_IMAGE(NAME, VIEW_TYPE, TASK_ACCESS) daxa::TaskImageUse<daxa::TaskImageAccess::TASK_ACCESS, daxa::ImageViewType::VIEW_TYPE> NAME{};
#define DAXA_DECL_TASK_USES_END() \
    }                             \
    uses = {};                    \
    }                             \
    ;

#endif
