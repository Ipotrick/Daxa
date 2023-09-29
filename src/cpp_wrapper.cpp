#include "cpp_wrapper.hpp"

#include "impl_core.hpp"
#include "impl_device.hpp"

#include <chrono>
#include <utility>

namespace daxa
{
    auto create_instance(InstanceInfo const & info) -> Instance
    {
        daxa_Instance instance;
        auto create_info = daxa_InstanceInfo{};
        if (info.enable_debug_utils)
        {
            create_info.flags |= DAXA_INSTANCE_FLAG_DEBUG_UTIL;
        }
        daxa_create_instance(&create_info, &instance);
        return Instance{ManagedPtr{new ImplInstance(instance, info)}};
    }
    Instance::Instance(ManagedPtr impl) : ManagedPtr(std::move(impl)) {}
    auto Instance::create_device(DeviceInfo const & device_info) -> Device
    {
        auto & impl = *as<ImplInstance>();
        auto c_info = daxa_DeviceInfo{};
        auto c_device = daxa_Device{};
        auto res = daxa_instance_create_device(impl.instance, &c_info, &c_device);
        DAXA_DBG_ASSERT_TRUE_M(res == DAXA_RESULT_SUCCESS, "Failed to create device");
        return {};
    }

    ImplInstance::ImplInstance(daxa_Instance a_instance, InstanceInfo a_info)
        : instance{a_instance}, info{std::move(a_info)}
    {
    }

    ImplInstance::~ImplInstance() // NOLINT(bugprone-exception-escape)
    {
        daxa_destroy_instance(instance);
    }
} // namespace daxa
