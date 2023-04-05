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
    struct ShaderTaskImageUse
    {
        std::string_view name = {};
        daxa::TaskImageAccess access = {};
        daxa::ImageMipArraySlice slice = {};
        daxa::ImageViewType view_type = {};
        daxa::usize offset = {};
    };

    struct ShaderTaskBufferUse
    {
        std::string_view name = {};
        daxa::TaskBufferAccess access = {};
        daxa::usize offset = {};
    };

    struct TaskShaderUses
    {
        std::vector<std::variant<ShaderTaskImageUse, ShaderTaskBufferUse, std::monostate>> list = {};
        u32 slot = {};
        u32 size = {};
    };

#define DAXA_INL_TASK_USES_BEGIN(NAME, SLOT) \
namespace daxa { \
    inline static const TaskShaderUses NAME = []() -> TaskShaderUses \
    { \
        TaskShaderUses uses = {}; \
        uses.slot = SLOT;
#define DAXA_INL_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) \
        /* must align buffer ptrs to 8 bytes */ \
        uses.size = ((uses.size + 7) / 8) * 8; \
        uses.list.push_back(ShaderTaskBufferUse{.name = #NAME, .access = daxa::TaskBufferAccess::TASK_ACCESS, .offset = uses.size}); \
        uses.size += sizeof(daxa::types::BufferDeviceAddress);
#define DAXA_INL_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) \
        uses.list.push_back(ShaderTaskImageUse{.name = #NAME, .access = daxa::TaskImageAccess::TASK_ACCESS, .slice = SLICE, .view_type = TYPE::view_type(), .offset = uses.size}); \
        uses.size += sizeof(daxa::types::ImageViewId);
#define DAXA_INL_TASK_USES_END() \
        return uses; \
    }(); \
}
}

#endif