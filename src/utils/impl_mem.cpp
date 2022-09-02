#if DAXA_BUILT_WITH_UTILS

#include <daxa/utils/mem.hpp>

namespace daxa
{
    MemoryUploader::MemoryUploader(Device device, usize capacity)
        : device{ std::move(device) }
        , gpu_timeline{ this->device.create_timeline_semaphore({ .initial_value = 0, .debug_name = "MemoryUploader" }) }
        , current_command_list{ this->device.create_command_list({ .debug_name = "MemoryUploader CommandList Nr. 0" }) }
        , capacity{ capacity }
        , claimed_sizes{ ClaimedSize{ .timeline_value = 1, .size = 0 } }
    { 

    }

    auto MemoryUploader::upload_to_buffer(BufferId dst_buffer, usize dst_offset, usize size) -> void*
    {
        usize src_offset = this->reserve_memoy(size);

        this->claimed_sizes.back().size += size;

        current_command_list.copy_buffer_to_buffer({
            .src_buffer = upload_buffer,
            .src_offset = src_offset,
            .dst_buffer = dst_buffer,
            .dst_offset = dst_offset,
            .size = size,
        });

        return reinterpret_cast<void*>(this->device.map_memory_as<u8>(this->upload_buffer) + src_offset);
    }

    auto MemoryUploader::get_commands() -> MemoryUploadCommandSubmitInfo
    {
        this->reclaim_unused_memory();

        this->current_command_list.complete();

        MemoryUploadCommandSubmitInfo ret
        {
            .command_list = this->current_command_list,
            .timeline = this->gpu_timeline,
            .timeline_signal_value = this->timeline_value,
        };

        this->current_command_list = this->device.create_command_list({
            .debug_name = std::string("MemoryUploader CommandList Nr. " + std::to_string(timeline_value)),
        });

        this->timeline_value += 1;

        claimed_sizes.push_back({
            .timeline_value = this->timeline_value,
            .size = 0,
        });

        return ret;
    }
    
    void MemoryUploader::reclaim_unused_memory()
    {
        usize gpu_timeline_value = gpu_timeline.value();
        while (true)
        {
            if (claimed_sizes.front().timeline_value <= gpu_timeline_value)
            {
                claimed_start = (claimed_start + claimed_sizes.front().size) % this->capacity;
                claimed_sizes.pop_front();
            }
        }
    }

    auto MemoryUploader::reserve_memoy(usize size) -> usize
    {
        usize start = this->claimed_start;
        usize old_size = this->claimed_size;
        this->claimed_size += size;
        DAXA_DBG_ASSERT_TRUE_M(this->claimed_size <= capacity, "exceeded MemoryUploader ring buffer capacity! A potential fix would be to increase the uploaders capacity.");
    
        return (start + old_size) % capacity;
    }
}

#endif // #if DAXA_BUILT_WITH_UTILS