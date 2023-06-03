#pragma once

#include <daxa/core.hpp>

#include <daxa/gpu_resources.hpp>

namespace daxa
{
    struct Device;
    struct MemoryBarrierInfo
    {
        Access src_access = AccessConsts::NONE;
        Access dst_access = AccessConsts::NONE;
    };

    auto to_string(MemoryBarrierInfo const & info) -> std::string;

    struct ImageBarrierInfo
    {
        Access src_access = AccessConsts::NONE;
        Access dst_access = AccessConsts::NONE;
        ImageLayout src_layout = ImageLayout::UNDEFINED;
        ImageLayout dst_layout = ImageLayout::UNDEFINED;
        ImageMipArraySlice image_slice = {};
        ImageId image_id = {};
    };

    auto to_string(ImageBarrierInfo const & info) -> std::string;

    struct SplitBarrierInfo
    {
        std::string name = {};
    };

    struct Device;

    struct SplitBarrierState
    {
        SplitBarrierState() = default;

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
