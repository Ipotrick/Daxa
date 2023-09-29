#include "impl_memory_block.hpp"

#include <utility>

#include "impl_device.hpp"

namespace daxa
{
    MemoryBlock::MemoryBlock(ManagedPtr impl) : ManagedPtr{ std::move(impl) }
    {
    }

    ImplMemoryBlock::ImplMemoryBlock(daxa_Device a_device, MemoryBlockInfo const & a_info, VmaAllocation a_alloc, VmaAllocationInfo a_alloc_info) 
        : device{ a_device }
        , info{ a_info }
        , allocation{ a_alloc }
        , alloc_info{ a_alloc_info }
    {
    }

    ImplMemoryBlock::~ImplMemoryBlock()
    {
        vmaFreeMemory(device->vma_allocator, this->allocation);
    }
} // namespace daxa
