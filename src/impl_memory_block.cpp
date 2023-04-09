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
        DAXA_ONLY_IF_THREADSAFETY(std::unique_lock const lock{device->main_queue_zombies_mtx});
        u64 const main_queue_cpu_timeline_value = DAXA_ATOMIC_FETCH(device->main_queue_cpu_timeline);
        device->main_queue_memory_block_zombies.push_front({
            main_queue_cpu_timeline_value,
            MemoryBlockZombie{
                .allocation = this->allocation,
            },
        });
    }
} // namespace daxa
