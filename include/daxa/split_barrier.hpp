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

    struct SplitBarrierState
    {
        SplitBarrierState(SplitBarrierState && other) noexcept;
        auto operator=(SplitBarrierState && other) noexcept -> SplitBarrierState &;
        ~SplitBarrierState();

        auto info() const -> SplitBarrierInfo const &;

      private:
        friend struct Device;
        friend struct CommandList;
        SplitBarrierState(ManagedWeakPtr a_impl_device, SplitBarrierInfo a_info);
        void cleanup();

        ManagedWeakPtr device = {};
        SplitBarrierInfo create_info = {};
        u64 data = {};
    };

    struct SplitBarrierSignalInfo
    {
        std::span<MemoryBarrierInfo> memory_barriers = {};
        std::span<ImageBarrierInfo> image_barriers = {};
        SplitBarrierState & split_barrier;
    };

    using SplitBarrierWaitInfo = SplitBarrierSignalInfo;
} // namespace daxa
