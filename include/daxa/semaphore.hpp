#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct BinarySemaphoreInfo
    {
        std::string debug_name = {};
    };

    struct BinarySemaphore : ManagedPtr
    {
        auto info() const -> BinarySemaphoreInfo const &;

      private:
        friend struct Device;
        explicit BinarySemaphore(ManagedPtr impl);
    };

    struct TimelineSemaphoreInfo
    {
        u64 initial_value = {};
        std::string debug_name = {};
    };

    struct TimelineSemaphore : ManagedPtr
    {
        auto info() const -> TimelineSemaphoreInfo const &;

        auto value() const -> u64;
        void set_value(u64 value);
        auto wait_for_value(u64 value, u64 timeout_nanos = ~0ull) -> bool;

      private:
        friend struct Device;
        explicit TimelineSemaphore(ManagedPtr impl);
    };
} // namespace daxa
