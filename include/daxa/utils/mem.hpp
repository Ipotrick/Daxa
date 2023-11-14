#pragma once

#if !DAXA_BUILT_WITH_UTILS_MEM
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_MEM CMake option enabled, or request the utils-mem feature in vcpkg"
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
        std::string name = {};
    };

    /// @brief Ring buffer based transfer memory allocator for easy and efficient cpu gpu communication.
    struct TransferMemoryPool
    {
        TransferMemoryPool(TransferMemoryPoolInfo a_info);
        TransferMemoryPool(TransferMemoryPool && other);
        TransferMemoryPool & operator=(TransferMemoryPool && other);
        ~TransferMemoryPool();

        struct Allocation
        {
            daxa::DeviceAddress device_address = {};
            void * host_address = {};
            u32 buffer_offset = {};
            usize size = {};
            u64 timeline_index = {};
        };
        // Returns nullopt if the allocation fails.
        auto allocate(u32 size, u32 alignment_requirement = 1) -> std::optional<Allocation>;
        // Returns current timeline index.
        auto timeline_value() const -> usize;
        // Returns timeline semaphore that needs to be signaled with the latest timeline value,
        // on a queue that uses memory from this pool.
        auto timeline_semaphore() -> TimelineSemaphore;
        auto buffer() const -> daxa::BufferId;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TransferMemoryPoolInfo const &;

      private:
        // Reclaim expired memory allocations.
        void reclaim_unused_memory();
        struct TrackedAllocation
        {
            usize timeline_index = {};
            u32 offset = {};
            u32 size = {};
        };

        TransferMemoryPoolInfo m_info = {};
        TimelineSemaphore gpu_timeline = {};

      private:
        u64 current_timeline_value = {};
        std::deque<TrackedAllocation> live_allocations = {};
        BufferId m_buffer = {};
        daxa::DeviceAddress buffer_device_address = {};
        void * buffer_host_address = {};
        u32 claimed_start = {};
        u32 claimed_size = {};
    };
} // namespace daxa
