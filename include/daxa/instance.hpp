#pragma once

#include <daxa/core.hpp>
#include <daxa/device.hpp>

namespace daxa
{
    struct InstanceInfo
    {
    };

    struct Instance : ManagedPtr
    {
        Instance() = default;

        auto create_device(DeviceInfo const & device_info) -> Device;

      private:
        friend auto create_instance(InstanceInfo const & info) -> Instance;
        explicit Instance(ManagedPtr impl);
    };

    auto create_instance(InstanceInfo const & info) -> Instance;
} // namespace daxa
