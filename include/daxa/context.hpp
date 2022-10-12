#pragma once

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    static inline void default_validation_callback([[maybe_unused]] daxa::MsgSeverity severity, daxa::MsgType, [[maybe_unused]] std::string_view msg)
    {
#if DAXA_VALIDATION
        switch (severity)
        {
        // clang-format off
        case daxa::MsgSeverity::VERBOSE: break;
        case daxa::MsgSeverity::INFO:    break;
        case daxa::MsgSeverity::WARNING: std::cout << '\n' << "[WARNING]: " << msg << '\n' << std::endl; DAXA_DBG_ASSERT_TRUE_M(false, "validation warning"); break;
        case daxa::MsgSeverity::FAILURE: std::cout << '\n' << "[FAILURE]: " << msg << '\n' << std::endl; DAXA_DBG_ASSERT_TRUE_M(false, "validation error"); break;
        // clang-format on
        default: std::cout << "[UNKNOWN]: " << msg << std::endl; break;
        }
#endif
    }

    struct ContextInfo
    {
        bool enable_validation = true;
        std::function<void(MsgSeverity, MsgType, std::string_view)> validation_callback = default_validation_callback;
    };

    struct Context : ManagedPtr
    {
        Context() = default;

        auto create_device(DeviceInfo const & device_info) -> Device;

      private:
        friend auto create_context(ContextInfo const & info) -> Context;
        explicit Context(ManagedPtr impl);
    };

    auto create_context(ContextInfo const & info) -> Context;
} // namespace daxa
