#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct SplitBarrierInfo
    {
        std::variant<MemoryBarrierInfo, ImageBarrierInfo> barrier;
        std::string debug_name = {};
    };

    struct Device;

    struct SplitBarrier
    {
        SplitBarrier(SplitBarrier&& other) noexcept;
        auto operator=(SplitBarrier&& other) noexcept -> SplitBarrier&;
        ~SplitBarrier();

        auto info() const -> SplitBarrierInfo const &;

      private:
        friend struct Device;
        friend struct CommandList;
        SplitBarrier(Device device, SplitBarrierInfo const& info);
        void cleanup();

        Device device;
        SplitBarrierInfo info = {};
        u64 data = {};
    };
} // namespace daxa
