#pragma once

#include <daxa/command_list.hpp>

#include "impl_core.hpp"
#include "impl_semaphore.hpp"
#include "impl_pipeline.hpp"

namespace daxa
{
    struct ImplDevice;

    static inline constexpr usize DEFERRED_DESTRUCTION_BUFFER_INDEX = 0;
    static inline constexpr usize DEFERRED_DESTRUCTION_IMAGE_INDEX = 1;
    static inline constexpr usize DEFERRED_DESTRUCTION_IMAGE_VIEW_INDEX = 2;
    static inline constexpr usize DEFERRED_DESTRUCTION_SAMPLER_INDEX = 3;
    static inline constexpr usize DEFERRED_DESTRUCTION_COUNT_MAX = 32;

    static inline constexpr usize COMMAND_LIST_BARRIER_MAX_BATCH_SIZE = 16;

    static inline constexpr usize COMMAND_LIST_COLOR_ATTACHMENT_MAX = 16;

    struct ImplCommandList
    {
        using InfoT = CommandListInfo;

        std::weak_ptr<ImplDevice> impl_device = {};
        CommandListInfo info = {};
        VkCommandBuffer vk_cmd_buffer = {};
        VkCommandPool vk_cmd_pool = {};
        bool recording_complete = true;
        std::array<VkMemoryBarrier2, COMMAND_LIST_BARRIER_MAX_BATCH_SIZE> memory_barrier_batch = {};
        std::array<VkImageMemoryBarrier2, COMMAND_LIST_BARRIER_MAX_BATCH_SIZE> image_barrier_batch = {};
        usize image_barrier_batch_count = 0;
        usize memory_barrier_batch_count = 0;
        std::array<VkPipelineLayout, PIPELINE_LAYOUT_COUNT> pipeline_layouts = {};
        std::array<std::pair<GPUResourceId, u8>, DEFERRED_DESTRUCTION_COUNT_MAX> deferred_destructions = {};
        usize deferred_destruction_count = {};

        void flush_barriers();

        ImplCommandList(std::weak_ptr<ImplDevice> device_impl);
        ~ImplCommandList();

        void initialize(CommandListInfo const & a_info);
        void reset();
    };
} // namespace daxa
