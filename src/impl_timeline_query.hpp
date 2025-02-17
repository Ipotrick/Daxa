#pragma once

#include <queue>

#include "impl_core.hpp"
#include <daxa/sync.hpp>


namespace daxa
{
    struct TimelineQueryPoolZombie
    {
        VkQueryPool vk_timeline_query_pool = {};
    };
} // namespace daxa

struct daxa_ImplTimelineQueryPool final : ImplHandle
{
    daxa_Device device = {};
    TimelineQueryPoolInfo info = {};
    std::string info_name = {};
    VkQueryPool vk_timeline_query_pool = {};

    static void zero_ref_callback(ImplHandle const * handle);
};
