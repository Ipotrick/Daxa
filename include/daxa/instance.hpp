#pragma once

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    struct InstanceFlagsProperties
    {
        using Data = u64;
    };
    using InstanceFlags = Flags<InstanceFlagsProperties>;
    struct InstanceFlagBits
    {
        static inline constexpr InstanceFlags NONE = {0x00000000};
        static inline constexpr InstanceFlags DEBUG_UTILS = {0x00000001};
    };

    struct InstanceInfo
    {
        InstanceFlags flags = InstanceFlagBits::DEBUG_UTILS;
        std::string_view engine_name;
        std::string_view app_name;
    };

    struct Instance : ManagedPtr
    {
        Instance() = default;

        auto create_device(DeviceInfo const & device_info) -> Device;

        auto info() const -> InstanceInfo const &;

      private:
        friend auto create_instance(InstanceInfo const & info) -> Instance;
        explicit Instance(ManagedPtr impl);
    };

    auto create_instance(InstanceInfo const & info) -> Instance;
} // namespace daxa
