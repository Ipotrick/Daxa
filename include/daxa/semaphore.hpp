#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct BinarySemaphoreInfo
    {
        std::string name = {};
    };

    struct BinarySemaphore : ManagedPtr
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
        std::string name = {};
    };

    struct TimelineSemaphore : ManagedPtr
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
} // namespace daxa
