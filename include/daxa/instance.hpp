#pragma once

#include <daxa/core.hpp>
#include <daxa/device.hpp>

#ifdef STREAMLINE_ENABLED
#include <sl.h>
#include <daxa/utils/streamline.hpp>
#endif // STREAMLINE_ENABLED

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
        static inline constexpr InstanceFlags PARENT_MUST_OUTLIVE_CHILD = {0x00000002};
    };

    struct InstanceInfo
    {
        InstanceFlags flags =
            InstanceFlagBits::DEBUG_UTILS |
            InstanceFlagBits::PARENT_MUST_OUTLIVE_CHILD;
        SmallString engine_name = "daxa";
        SmallString app_name = "daxa app";
#ifdef STREAMLINE_ENABLED
        // Streamline configuration
        bool enable_streamline = false;
        Span<sl::Feature const> sl_features = {};
#endif // STREAMLINE_ENABLED
    };

    struct DAXA_EXPORT_CXX Instance final : ManagedPtr<Instance, daxa_Instance>
    {
        Instance() = default;

#if !DAXA_REMOVE_DEPRECATED
        [[deprecated("Use create_device_2 instead, API:3.0")]] [[nodiscard]] auto create_device(DeviceInfo const & device_info) -> Device;
#endif
        [[nodiscard]] auto create_device_2(DeviceInfo2 const & device_info) -> Device;

        /// Convenience function to pick a physical device.
        /// Picks first supported device that satisfies the given device info and desired implicit features.
        [[nodiscard]] auto choose_device(ImplicitFeatureFlags desired_features, DeviceInfo2 const & base_info) -> DeviceInfo2;

        [[nodiscard]] auto list_devices_properties() -> std::span<DeviceProperties const>;

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        [[nodiscard]] auto info() const -> InstanceInfo const &;

#ifdef STREAMLINE_ENABLED
        [[nodiscard]] auto streamline() const -> StreamlineContext const &;
#endif 

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    [[nodiscard]] DAXA_EXPORT_CXX auto create_instance(InstanceInfo const & info) -> Instance;
} // namespace daxa
