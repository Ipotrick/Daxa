#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct MemoryRequirements
    {
        u64 size = {};
        u64 alignment = {};
        u32 memory_type_bits = {};
    };

    struct MemoryBlockInfo
    {
        MemoryRequirements requirements = {};
        MemoryFlags flags = {};
    };

    struct MemoryBlock : ManagedPtr
    {
        MemoryBlock() = default;

      private:
        friend struct ImplDevice;
        friend struct Device;
        MemoryBlock(ManagedPtr impl);
    };

    using AutoAllocInfo = MemoryFlags;

    struct ManualAllocInfo
    {
        MemoryBlock memory_block = {};
        usize offset = {};
    };

    using AllocateInfo = std::variant<AutoAllocInfo, ManualAllocInfo>;
} // namespace daxa
