#pragma once

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    struct ContextInfo
    {
        bool enable_validation = false;
        std::function<void(MsgSeverity, MsgType, std::string_view)> validation_callback;
    };

    struct Context : Handle
    {
        auto create_device(DeviceInfo const &device_info) -> Device;
        auto create_default_device() -> Device;

      private:
        friend auto create_context(ContextInfo const & info) -> Context;
        Context(std::shared_ptr<void> impl);
    };

    auto create_context(ContextInfo const & info) -> Context;
} // namespace daxa
