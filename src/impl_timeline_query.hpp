#pragma once

#include <queue>

#include <daxa/sync.hpp>

#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    struct TimelineQueryPoolZombie
    {
        VkQueryPool vk_timeline_query_pool = {};
    };

    struct ImplTimelineQueryPool final : ManagedSharedState
    {
        TimelineQueryPoolInfo info = {};
        VkQueryPool vk_timeline_query_pool = {};

        ImplTimelineQueryPool(daxa_Device a_device, TimelineQueryPoolInfo a_info);
        ~ImplTimelineQueryPool();

      private:
        friend struct TimelineQueryPool;
        daxa_Device device = {};
    };
} // namespace daxa
