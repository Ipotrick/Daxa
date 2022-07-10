#pragma once

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    struct ContextInfo
    {
        bool enable_validation = false;
    };

    struct Context : Handle
    {
        friend auto create_context(ContextInfo const & info) -> Context;

        auto create_device(DeviceInfo const & info) -> Device;

      private:
        Context() {}
    };

    auto create_context(ContextInfo const & info) -> Context;
} // namespace daxa
