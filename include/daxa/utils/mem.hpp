#pragma once

#if !DAXA_BUILT_WITH_UTILS_MEM
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_MEM CMake option enabled, or request the utils-mem feature in vcpkg"
#endif

#include <daxa/core.hpp>
#include <daxa/device.hpp>

#include <deque>

namespace daxa
{
    struct RingBufferInfo
    {
        Device device = {};
        u32 capacity = 1 << 25;
        bool prefer_device_memory = true;
        std::string name = {};
    };

    using TransferMemoryPoolInfo = RingBufferInfo;

    /// @brief Ring buffer based transfer memory allocator for easy and efficient cpu gpu communication.
    struct RingBuffer
    {
        DAXA_EXPORT_CXX RingBuffer(RingBufferInfo a_info);
        DAXA_EXPORT_CXX RingBuffer(RingBuffer && other);
        DAXA_EXPORT_CXX RingBuffer & operator=(RingBuffer && other);
        DAXA_EXPORT_CXX ~RingBuffer();

        struct Allocation
        {
            daxa::DeviceAddress device_address = {};
            void * host_address = {};
            u32 buffer_offset = {};
            usize size = {};
            u64 submit_index = {};
        };
        /// @return returns an Allocation if successful, otherwise returns std::nullopt.
        DAXA_EXPORT_CXX auto allocate(u32 size, u32 alignment_requirement = 16 /* 16 is a save default for most gpu data*/) -> std::optional<Allocation>;
        /// @brief  Allocates a section of a buffer with the size of T, writes the given T to the allocation.
        /// @return returns an Allocation if successful, otherwise returns std::nullopt.
        template<typename T>
        auto allocate_fill(T const & value, u32 alignment_requirement = alignof(T)) -> std::optional<Allocation>
        {
            auto allocation_o = allocate(sizeof(T), alignment_requirement);
            if (allocation_o.has_value())
            {
                *reinterpret_cast<T*>(allocation_o->host_address) = value;
                return allocation_o.value();
            }
            return std::nullopt;
        }
        
        DAXA_EXPORT_CXX auto buffer() const -> daxa::BufferId;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        DAXA_EXPORT_CXX auto info() const -> RingBufferInfo const &;

        /// @brief Marks ALL allocations made prior to calling this function as reclaimable.
        ///        Memory will be reclaimed ONLY AFTER all currently pending submits have completed execution on the GPU.
        ///        Easiest way to use this is to call it at the end of a frame, so that all allocations made during the frame can be reclaimed.
        DAXA_EXPORT_CXX void reuse_memory_after_pending_submits();

      private:
        // Reclaim expired memory allocations.
        DAXA_EXPORT_CXX void reclaim_memory();
        struct TrackedAllocation
        {
            usize submit_index = {};
            u32 offset = {};
            u32 size = {};
        };

        // used to mark allocations and when we can free them.
        // We can free all allocations before this index.
        u64 reclaim_submit_index = {};

        RingBufferInfo m_info = {};
        std::deque<TrackedAllocation> live_allocations = {};
        BufferId m_buffer = {};
        daxa::DeviceAddress buffer_device_address = {};
        void * buffer_host_address = {};
        u32 claimed_start = {};
        u32 claimed_size = {};
    };
    
    using TransferMemoryPool = RingBuffer;
} // namespace daxa
