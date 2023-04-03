#pragma once
#include "../daxa.inl"

#if DAXA_SHADER
#define DAXA_TASK_USES_BEGIN(Name) struct Name {
#define DAXA_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) TYPE NAME;
#define DAXA_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) TYPE NAME;
#define DAXA_TASK_USES_END() };
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
        daxa::usize offset = {};
    };

    struct ShaderTaskBufferUse
    {
        std::string_view name = {};
        daxa::TaskBufferAccess access = {};
        daxa::usize offset = {};
    };

    struct ShaderTaskUses
    {
        std::vector<std::variant<ShaderTaskImageUse, ShaderTaskBufferUse, std::monostate>> list = {};
        u32 size = {};
    };

#define DAXA_TASK_USES_BEGIN(NAME) \
namespace daxa { \
    inline static const ShaderTaskUses NAME = []() -> ShaderTaskUses \
    { \
        ShaderTaskUses uses = {};
#define DAXA_TASK_USE_IMAGE(NAME, TYPE, TASK_ACCESS, SLICE) \
        uses.list.push_back(ShaderTaskImageUse{.name = #NAME, .access = daxa::TaskImageAccess::TASK_ACCESS, .slice = SLICE, .offset = uses.size}); \
        uses.size += sizeof(daxa::types::ImageViewId);
#define DAXA_TASK_USE_BUFFER(NAME, TYPE, TASK_ACCESS) \
        uses.list.push_back(ShaderTaskBufferUse{.name = #NAME, .access = daxa::TaskBufferAccess::TASK_ACCESS, .offset = uses.size}); \
        uses.size += sizeof(daxa::types::BufferDeviceAddress);
#define DAXA_TASK_USES_END() \
        return uses; \
    }(); \
}
}

#endif