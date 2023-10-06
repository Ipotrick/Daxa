#pragma once

#include <daxa/types.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplMemoryBlock : daxa_ImplHandle
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
