#pragma once

#include <daxa/types.hpp>
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

    struct BinarySemaphoreInfo
    {
        std::string_view name = {};
    };

    struct BinarySemaphore final : ManagedPtr
    {
        BinarySemaphore() = default;

        auto info() const -> BinarySemaphoreInfo const &;

      private:
        friend struct Device;
        friend struct ImplSwapchain;
        explicit BinarySemaphore(ManagedPtr impl);
    };

    struct TimelineSemaphoreInfo
    {
        u64 initial_value = {};
        std::string_view name = {};
    };

    struct TimelineSemaphore final : ManagedPtr
    {
        TimelineSemaphore() = default;

        auto info() const -> TimelineSemaphoreInfo const &;

        auto value() const -> u64;
        void set_value(u64 value);
        auto wait_for_value(u64 value, u64 timeout_nanos = ~0ull) -> bool;

      private:
        friend struct Device;
        friend struct ImplSwapchain;
        explicit TimelineSemaphore(ManagedPtr impl);
    };

    struct EventInfo
    {
        std::string_view name = {};
    };

    struct Event final : ManagedPtr
    {
        Event() = default;

        auto info() const -> EventInfo const &;

      private:
        friend struct CommandList;
        friend struct Device;
        Event(ManagedPtr impl);
    };

    struct EventSignalInfo
    {
        std::span<MemoryBarrierInfo> memory_barriers = {};
        std::span<ImageBarrierInfo> image_barriers = {};
        Event & split_barrier;
    };

    using EventWaitInfo = EventSignalInfo;
} // namespace daxa