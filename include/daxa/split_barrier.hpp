#pragma once

#include <daxa/core.hpp>

#include <daxa/gpu_resources.hpp>

namespace daxa
{
    struct Device;
    struct MemoryBarrierInfo
    {
        Access awaited_pipeline_access = AccessConsts::NONE;
        Access waiting_pipeline_access = AccessConsts::NONE;
    };

    struct ImageBarrierInfo
    {
        Access awaited_pipeline_access = AccessConsts::NONE;
        Access waiting_pipeline_access = AccessConsts::NONE;
        ImageLayout before_layout = ImageLayout::UNDEFINED;
        ImageLayout after_layout = ImageLayout::UNDEFINED;
        ImageMipArraySlice image_slice = {};
        ImageId image_id = {};
    };

    struct SplitBarrierInfo
    {
        std::string debug_name = {};
    };

    struct Device;

    struct SplitBarrier
    {
        SplitBarrier(SplitBarrier && other) noexcept;
        auto operator=(SplitBarrier && other) noexcept -> SplitBarrier &;
        ~SplitBarrier();

        auto info() const -> SplitBarrierInfo const &;

      private:
        friend struct Device;
        friend struct CommandList;
        SplitBarrier(ManagedWeakPtr device, SplitBarrierInfo info);
        void cleanup();

        ManagedWeakPtr device = {};
        SplitBarrierInfo create_info = {};
        u64 data = {};
    };

    struct SplitBarrierStartInfo
    {
        std::span<MemoryBarrierInfo> memory_barriers = {};
        std::span<ImageBarrierInfo> image_barriers = {};
        SplitBarrier & split_barrier;
    };

    using SplitBarrierEndInfo = SplitBarrierStartInfo;
} // namespace daxa
