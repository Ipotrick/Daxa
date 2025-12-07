#pragma once

#include <daxa/types.hpp>
#include <daxa/gpu_resources.hpp>

namespace daxa
{
    struct Device;

    struct BarrierInfo
    {
        Access src_access = AccessConsts::NONE;
        Access dst_access = AccessConsts::NONE;
    };

#if !DAXA_REMOVE_DEPRECATED
    using MemoryBarrierInfo [[deprecated("Use BarrierInfo instead; API:3.2")]] = BarrierInfo;
#endif

    [[nodiscard]] auto to_string(BarrierInfo const & info) -> std::string;

#if !DAXA_REMOVE_DEPRECATED
    struct [[deprecated("Use ImageBarrierInfo instead; API:3.2")]] ImageMemoryBarrierInfo
    {
        Access src_access = AccessConsts::NONE;
        Access dst_access = AccessConsts::NONE;
        ImageLayout src_layout = ImageLayout::UNDEFINED;
        ImageLayout dst_layout = ImageLayout::UNDEFINED;
        [[deprecated("Ignored parameter, whole image will be transitioned; API:3.2")]] ImageMipArraySlice image_slice = {};
        ImageId image_id = {};
    };

    [[nodiscard]] [[deprecated("Use ImageBarrierInfo instead; API:3.2")]] DAXA_EXPORT_CXX auto to_string(ImageMemoryBarrierInfo const & info) -> std::string;
#endif

    enum struct ImageLayoutOperation
    {
        NONE = 0,
        TO_GENERAL = 1,
        TO_PRESENT_SRC = 2,
    };

    struct ImageBarrierInfo
    {
        Access src_access = AccessConsts::NONE;
        Access dst_access = AccessConsts::NONE;
        ImageId image_id = {};
        ImageLayoutOperation layout_operation = {};
    };

    [[nodiscard]] DAXA_EXPORT_CXX auto to_string(ImageBarrierInfo const & info) -> std::string;

    struct BinarySemaphoreInfo
    {
        SmallString name = {};
    };

    struct DAXA_EXPORT_CXX BinarySemaphore final : ManagedPtr<BinarySemaphore, daxa_BinarySemaphore>
    {
        BinarySemaphore() = default;

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the device is destroyed.
        /// @return reference to info of object.
        [[nodiscard]] auto info() const -> BinarySemaphoreInfo const &;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TimelineSemaphoreInfo
    {
        u64 initial_value = {};
        SmallString name = {};
    };

    struct DAXA_EXPORT_CXX TimelineSemaphore final : ManagedPtr<TimelineSemaphore, daxa_TimelineSemaphore>
    {
        TimelineSemaphore() = default;

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        [[nodiscard]] auto info() const -> TimelineSemaphoreInfo const &;

        [[nodiscard]] auto value() const -> u64;
        void set_value(u64 value);
        [[nodiscard]] auto wait_for_value(u64 value, u64 timeout_nanos = ~0ull) -> bool;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct EventInfo
    {
        SmallString name = {};
    };

    struct DAXA_EXPORT_CXX Event final : ManagedPtr<Event, daxa_Event>
    {
        Event() = default;

        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        [[nodiscard]] auto info() const -> EventInfo const &;

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct EventSignalInfo
    {
        daxa::Span<BarrierInfo const> barriers = {};
        daxa::Span<ImageBarrierInfo const> image_barriers = {};
        Event & event;
    };

    using EventWaitInfo = EventSignalInfo;
} // namespace daxa