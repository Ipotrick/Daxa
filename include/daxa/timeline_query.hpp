#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct TimelineQueryPoolInfo
    {
        u32 query_count = {};
        std::string debug_name = {};
    };

    struct TimelineQueryPool : ManagedPtr
    {
        auto info() const -> TimelineQueryPoolInfo const &;

        auto get_query_results(u32 start_index, u32 count) -> std::vector<u64>;

      private:
        friend struct Device;
        TimelineQueryPool(ManagedPtr impl);
    };
} // namespace daxa
