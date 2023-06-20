#include "impl_memory_block.hpp"

#include <utility>

#include "impl_device.hpp"

namespace daxa
{
    MemoryBlock::MemoryBlock(ManagedPtr impl) : ManagedPtr{ std::move(impl) }
    {
    }

    ImplMemoryBlock::ImplMemoryBlock(ManagedWeakPtr a_device, MemoryBlockInfo const & a_info, VmaAllocation a_alloc, VmaAllocationInfo a_alloc_info) 
        : impl_device{ a_device }
        , info{ a_info }
        , allocation{ a_alloc }
        , alloc_info{ a_alloc_info }
    {
    }

    ImplMemoryBlock::~ImplMemoryBlock()
    {
        auto * device = this->impl_device.as<ImplDevice>();
        vmaFreeMemory(device->vma_allocator, this->allocation);
    }
} // namespace daxa
