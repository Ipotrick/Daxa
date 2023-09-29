#pragma once

#include <daxa/memory_block.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplMemoryBlock : ManagedSharedState
    {
        ImplMemoryBlock(daxa_Device a_device, MemoryBlockInfo const & info, VmaAllocation allocation, VmaAllocationInfo alloc_info);
        ~ImplMemoryBlock();

        daxa_Device device = {};
        MemoryBlockInfo info = {};
        VmaAllocation allocation = {};
        VmaAllocationInfo alloc_info = {};
    };

    struct MemoryBlockZombie
    {
        VmaAllocation allocation = {};
    };
} // namespace daxa
