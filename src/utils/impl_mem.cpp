#if DAXA_BUILT_WITH_UTILS

#include <daxa/utils/mem.hpp>

namespace daxa
{
    TransferMemoryPool::TransferMemoryPool(TransferMemoryPoolInfo const & info)
        : info{info},
          gpu_timeline{this->info.device.create_timeline_semaphore({
              .initial_value = {},
              .debug_name = this->info.debug_name + ": timeline semaphore",
          })},
          buffer{this->info.device.create_buffer({
              .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
              .size = this->info.capacity,
              .debug_name = this->info.debug_name + ": buffer",
          })}
    {
    }

    auto TransferMemoryPool::allocate(u32 allocation_size) -> std::optional<TransferMemoryPool::Allocation>
    {
        // Firstly, test if there is enough continuous space left to allocate.
        bool tail_allocation_possible =
            (this->claimed_start + this->claimed_size + allocation_size) < this->info.capacity;
        // When there is no tail space left, it may be the case that we can place the allocation at offset 0.
        // Illustration: |XXX ## |; "X": new allocation; "#": used up space; " ": free space.
        bool zero_offset_allocation_possible =
            allocation_size < this->claimed_start &&
            ((this->claimed_start + this->claimed_size) < this->info.capacity);
        if (!tail_allocation_possible && !zero_offset_allocation_possible)
        {
            this->reclaim_unused_memory();
            tail_allocation_possible =
                (this->claimed_start + this->claimed_size + allocation_size) < this->info.capacity;
            zero_offset_allocation_possible =
                allocation_size < this->claimed_start &&
                ((this->claimed_start + this->claimed_size) < this->info.capacity);
            if (!tail_allocation_possible && !zero_offset_allocation_possible)
            {
                return std::nullopt;
            }
        }
        current_timeline_value += 1;
        u32 actual_allocation_size = {};
        u32 allocation_offset = {};
        if (tail_allocation_possible)
        {
            actual_allocation_size = allocation_size;
            allocation_offset = (this->claimed_start + this->claimed_size) % this->info.capacity;
        }
        else // Zero offset allocation.
        {
            u32 left_tail_space = this->info.capacity - (this->claimed_start + this->claimed_size);
            actual_allocation_size = allocation_size + left_tail_space;
            allocation_offset = 0;
        }
        this->claimed_size += actual_allocation_size;
        live_allocations.push_back(TrackedAllocation{
            .timeline_index = this->current_timeline_value,
            .size = actual_allocation_size,
        });
        return Allocation{
            .device_address = this->buffer_device_address + allocation_offset,
            .host_address = reinterpret_cast<void *>(this->buffer_host_address + allocation_offset),
            .size = allocation_size,
            .timeline_index = this->current_timeline_value,
        };
    }

    auto TransferMemoryPool::timeline_value() const -> usize
    {
        return this->current_timeline_value;
    }

    void TransferMemoryPool::reclaim_unused_memory()
    {
        auto const current_gpu_timeline_value = this->gpu_timeline.value();
        while (!live_allocations.empty() && live_allocations.front().timeline_index <= current_gpu_timeline_value)
        {
            this->claimed_start = (this->claimed_start + live_allocations.front().size) % this->info.capacity;
            live_allocations.pop_front();
        }
    }

    auto TransferMemoryPool::get_info() const -> TransferMemoryPoolInfo const &
    {
        return this->info;
    }
} // namespace daxa

#endif // #if DAXA_BUILT_WITH_UTILS
