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
        ~BinarySemaphore();

      private:
        friend struct Device;
        BinarySemaphore(std::shared_ptr<void> impl);
    };
} // namespace daxa
