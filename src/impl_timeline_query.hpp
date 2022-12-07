#pragma once

#include <queue>

#include <daxa/timeline_query.hpp>

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

        ImplTimelineQueryPool(ManagedWeakPtr a_impl_device, TimelineQueryPoolInfo a_info);
        ~ImplTimelineQueryPool();

      private:
        friend struct TimelineQueryPool;
        ManagedWeakPtr impl_device = {};
    };
} // namespace daxa
