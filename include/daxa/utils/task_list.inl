#pragma once
#include "../daxa.inl"

#if DAXA_SHADER
#define DAXA_INL_TASK_USES_BEGIN(Name, SLOT) DAXA_CONSTANT_BUFFER(SLOT) NAME {
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) TYPE NAME;
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) TYPE NAME;
#define DAXA_INL_TASK_USES_END() };
#else
#include <string_view>
#include <vector>
#include <variant>

#include "task_list_types.hpp"

namespace daxa
{
    struct ShaderTaskImageUseInit
    {
        std::string_view name = {};
        daxa::TaskImageAccess access = {};
        daxa::ImageMipArraySlice slice = {};
        daxa::ImageViewType view_type = {};
        daxa::usize offset = {};
    };

    struct ShaderTaskBufferUseInit
    {
        std::string_view name = {};
        daxa::TaskBufferAccess access = {};
        daxa::usize offset = {};
    };

    struct TaskShaderUses
    {
        std::vector<std::variant<ShaderTaskImageUseInit, ShaderTaskBufferUseInit, std::monostate>> list = {};
        u32 slot = {};
        u32 size = {};
    };

}

#define DAXA_INL_TASK_USES_BEGIN(NAME, SLOT) \
    struct NAME : public daxa::TaskUses<NAME, SLOT> \
    {
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) \
        daxa::TaskBufferUse NAME = {{.access = daxa::TaskBufferAccess::TASK_ACCESS}};
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) \
        daxa::TaskImageUse NAME = {{.access = daxa::TaskImageAccess::TASK_ACCESS, .slice = SLICE, .view_type = TYPE::view_type()}};
#define DAXA_INL_TASK_USES_END() \
    };

#endif