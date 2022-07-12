#pragma once

#include <daxa/core.hpp>

namespace daxa
{
    struct BinarySemaphoreInfo
    {
        std::string debug_name = {};
    };

    struct BinarySemaphore : Handle
    {
      private:
        friend struct Device;
        BinarySemaphore(std::shared_ptr<void> impl);
    };

    struct SemaphoreInfo
    {
        std::string debug_name = {};
    };

    struct Semaphore
    {
        u64 initialValue = 0;
        std::string debug_name = {};
    };

    // struct BinarySemaphore : Handle
    // {
    //     auto value() const -> u64 const &;
    //     void set_value(u64 new_value);
    //     auto wait_for_value(u64 awaited_value, std::chrono::microseconds timeout) -> bool;
    //   private:
    //     friend struct Device;
    //     BinarySemaphore(std::shared_ptr<void> impl);
    // };
} // namespace daxa
