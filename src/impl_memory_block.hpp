#pragma once

#include <daxa/memory_block.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplMemoryBlock : ManagedSharedState
    {
        ImplMemoryBlock(ManagedWeakPtr device, MemoryBlockInfo const & info, VmaAllocation allocation, VmaAllocationInfo alloc_info);
        ~ImplMemoryBlock();

        ManagedWeakPtr impl_device = {};
        MemoryBlockInfo info = {};
        VmaAllocation allocation = {};
        VmaAllocationInfo alloc_info = {};
    };

    struct MemoryBlockZombie
    {
        VmaAllocation allocation = {};
    };
} // namespace daxa
