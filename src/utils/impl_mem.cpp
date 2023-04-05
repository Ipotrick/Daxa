#if DAXA_BUILT_WITH_UTILS_MEM

#include <daxa/utils/mem.hpp>
#include <utility>

namespace daxa
{
    TransferMemoryPool::TransferMemoryPool(TransferMemoryPoolInfo a_info)
        : info{std::move(a_info)},
          gpu_timeline{this->info.device.create_timeline_semaphore({
              .initial_value = {},
              .name = this->info.name + ": timeline semaphore",
          })},
          buffer{this->info.device.create_buffer({
              .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
              .size = this->info.capacity,
              .name = this->info.name + ": buffer",
          })},
          buffer_device_address{this->info.device.get_device_address(this->buffer)},
          buffer_host_address{this->info.device.get_host_address(this->buffer)}
    {
    }

    TransferMemoryPool::~TransferMemoryPool()
    {
        this->info.device.destroy_buffer(this->buffer);
    }

    auto TransferMemoryPool::allocate(u32 allocation_size) -> std::optional<TransferMemoryPool::Allocation>
    {
        // Two allocations are possible:
        // Tail allocation is when the allocation is placed directly at the end of all other allocations.
        // Zero offset allocation is possible when there is not enough space left at the tail BUT there is enough space from 0 up to the start of the other allocations.
        auto calc_tail_allocation_possible = [&]()
        {
            u32 const tail = (this->claimed_start + this->claimed_size) % this->info.capacity;
            bool const wrapped = this->claimed_start + this->claimed_size > this->info.capacity;
            u32 const end = wrapped ? this->claimed_start : this->info.capacity;
            return tail + allocation_size <= end;
        };
        auto calc_zero_offset_allocation_possible = [&]()
        {
            return this->claimed_start + this->claimed_size <= this->info.capacity;
        };
        // Firstly, test if there is enough continuous space left to allocate.
        bool tail_allocation_possible = calc_tail_allocation_possible();
        // When there is no tail space left, it may be the case that we can place the allocation at offset 0.
        // Illustration: |XXX ## |; "X": new allocation; "#": used up space; " ": free space.
        bool zero_offset_allocation_possible = calc_zero_offset_allocation_possible();
        if (!tail_allocation_possible && !zero_offset_allocation_possible)
        {
            this->reclaim_unused_memory();
            tail_allocation_possible = calc_tail_allocation_possible();
            zero_offset_allocation_possible = calc_zero_offset_allocation_possible();
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
            u32 const left_tail_space = this->info.capacity - (this->claimed_start + this->claimed_size);
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
            .host_address = reinterpret_cast<void *>(reinterpret_cast<u8 *>(this->buffer_host_address) + allocation_offset),
            .buffer_offset = allocation_offset,
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
            this->claimed_size -= live_allocations.front().size;
            live_allocations.pop_front();
        }
    }

    auto TransferMemoryPool::get_timeline_semaphore() -> TimelineSemaphore
    {
        return this->gpu_timeline;
    }

    auto TransferMemoryPool::get_info() const -> TransferMemoryPoolInfo const &
    {
        return this->info;
    }

    auto TransferMemoryPool::get_buffer() const -> daxa::BufferId
    {
        return this->buffer;
    }
} // namespace daxa

#endif
