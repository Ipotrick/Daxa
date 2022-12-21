#pragma once

#if !DAXA_BUILT_WITH_UTILS
#error "[package management error] You must build Daxa with the UTILS option enabled"
#endif

#include <daxa/core.hpp>
#include <daxa/device.hpp>

#include <deque>

namespace daxa
{
    struct TransferMemoryPoolInfo
    {
        Device device = {};
        u32 capacity = 1 << 25;
        bool use_bar_memory = {};
        std::string debug_name = {};
    };

    /// @brief Ring buffer based transfer memory allocator for easy and efficient cpu gpu communication.
    struct TransferMemoryPool
    {
        TransferMemoryPool(TransferMemoryPoolInfo const & info);

        struct Allocation
        {
            daxa::BufferDeviceAddress device_address = {};
            void * host_address = {};
            usize size = {};
            u64 timeline_index = {};
        };
        // Returns nullopt if the allocation fails.
        auto allocate(u32 size) -> std::optional<Allocation>;
        // Returns current timeline index.
        auto timeline_value() const -> usize;
        // Returns timeline semaphore that needs to be signaled with the latest timeline value,
        // on a queue that uses memory from this pool.
        auto get_timeline_semaphore() -> TimelineSemaphore;
        auto get_info() const -> TransferMemoryPoolInfo const &;

      private:
        // Reclaim expired memory allocations.
        void reclaim_unused_memory();
        struct TrackedAllocation
        {
            usize timeline_index = {};
            u32 size = {};
        };

        TransferMemoryPoolInfo info = {};

      public:
        TimelineSemaphore gpu_timeline = {};

      private:
        u64 current_timeline_value = {};
        std::deque<TrackedAllocation> live_allocations = {};
        BufferId buffer = {};
        daxa::BufferDeviceAddress buffer_device_address = {};
        u8 * buffer_host_address = {};
        u32 claimed_start = {};
        u32 claimed_size = {};
    };
} // namespace daxa
