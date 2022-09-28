#pragma once

#if !DAXA_BUILT_WITH_UTILS
#error "[package management error] You must build Daxa with the UTILS option enabled"
#endif

#include <daxa/core.hpp>
#include <daxa/device.hpp>

#include <deque>

namespace daxa
{
    struct MemoryUploadCommandSubmitInfo
    {
        CommandList command_list;
        TimelineSemaphore timeline;
        u64 timeline_signal_value;
    };

    // Not threadsafe
    // No automatic gpu synchronization
    struct MemoryUploader
    {
        MemoryUploader(Device device, usize capacity = 1 << 25);

        auto upload_to_buffer(BufferId dst_buffer, usize dst_offset, usize upload_size) -> void *;
        auto get_commands() -> MemoryUploadCommandSubmitInfo;
        void reclaim_unused_memory();

      private:
        auto reserve_memory(usize size) -> usize;

        struct ClaimedSize
        {
            usize timeline_value = {};
            usize size = {};
        };

        Device device;
        TimelineSemaphore gpu_timeline;
        CommandList current_command_list = {};
        usize timeline_value = 1;
        std::deque<ClaimedSize> claimed_sizes = {};
        BufferId upload_buffer = {};
        usize capacity;
        usize claimed_start = {};
        usize claimed_size = {};
    };
} // namespace daxa
